.intel_syntax noprefix
.code16

#define FAT_TABLE 0x9000
#define FAT_TABLE_SIZE_IN_SECTORS ((0xf000 - 0x9000) / 512)
#define ELF_HEADER 0xf000
#define SECTORS_PER_CLUSTER 8

#define BSS_ADDR 0x500
#define BSS_SIZE 0x100
#define KERNEL_FIRST_CLUSTER 0x500 /* word */
#define FIRST_CLUSTER_SECTOR 0x502 /* word */
#define DRIVER_NUMBER        0x504 /* byte */

// Disk address Packet
// https://en.wikipedia.org/wiki/INT_13H#INT_13h_AH=42h:_Extended_Read_Sectors_From_Drive
#define DAP          0x510
#define DAP_SIZE     0x510 /* byte */
#define DAP_NUM      0x512 /* word */
#define DAP_DEST     0x514 /* word */
#define DAP_DEST_SEG 0x516 /* word */
#define DAP_SECTOR   0x518 /* quad */

.globl boot
boot:
    cli
    cld
    mov ax, 0
    mov ds, ax
    mov ss, ax
    mov sp, 0x7c00
    mov di, BSS_ADDR
    mov cx, BSS_SIZE
    rep stosb

    // DL register holds the drive index set by BIOS.
    mov [DRIVER_NUMBER], dl

    // Initialize the serial port.
    mov ah, 0x00
    mov al, 0xe3    // baud_rate=9600, no parity,
                    // one stop bit, one character is 8bits
    mov dx, 0x0000  // The COM port.
    int 0x14

    mov byte ptr [DAP_SIZE], 0x10

    // Look for the dictory entry.
    lea ax, kernel_filename
    mov dx, 1900      //  The sector number.
    mov cx, 11        // filename (8 bytes) + extension (3 bytes)
    call look_for_signature

    // Get the first cluster number.
    // di points to the the beginning of file name field which
    // is the beginning of a directory entry.
    mov ax, [di + 0x1a] // assuming that higher 16-bits are zero
    mov [KERNEL_FIRST_CLUSTER], ax

    // Look for BIOS Parameter Block
    lea ax, bpb_jump_code
    mov dx, 1      //  The sector number.
    mov cx, 3
    call look_for_signature

    // (bpb sec) + (# of rsvd sec) + (# of fat) * (sec per fat)
    mov bx, [di + 0x0e] // # of reserved sectors
    xor cx, cx
    mov cl, [di + 0x10] // # of FATs
    mov ax, [di + 0x24] // # sectors per FAT
    add bx, dx // the beginning of BPB
    mul cx
    add bx, ax
    mov [FIRST_CLUSTER_SECTOR], bx

    // Look for the FAT ID and endianess marker. (F0 FF FF FF)
    lea ax, fat_table_signature
    mov dx, 1      //  The sector number.
    mov cx, 4
    call look_for_signature

    // Load the FAT table.
    mov word ptr [DAP_SECTOR], dx
    mov word ptr [DAP_DEST], FAT_TABLE
    mov word ptr [DAP_NUM], FAT_TABLE_SIZE_IN_SECTORS
    call read_sector

    // Load ELF header.
    mov di, ELF_HEADER
    mov si, 0
    mov cx, 1
    call read_kernel_file

    // TODO: check header

    // # of program headers.
    mov dx, [ELF_HEADER + 0x38]
    mov cl, 3 // assuming that 8 sectors per cluster
              // TODO: log2(num of sectors in a cluster)
    add cl, 9 // assuming that offset is multiple of 512

    // Load segments.
    mov bx, ELF_HEADER + 64 // the first program header offset

#define PT_LOAD 1
load_segment:
    mov dx, [bx]
    cmp dx, PT_LOAD
    jne 1f

    mov si, [bx + 0x08] // offset in the file
    mov edi, [bx + 0x10] // virtual address
    mov dx, [bx + 0x20] // size
    shr si, cl // assuming that offset is multiple of cluster
    shr dx, cl
    inc dx // in case the segment size is not multiple of 512
    mov cx, dx
    call read_kernel_file

1:
    sub dx, 1
    jz 2f
    add ax, 0x38
    jmp load_segment

2:
    // Jump into the kernel.
    mov edx, [ELF_HEADER + 0x18]
    mov eax, edx
    shr eax, 4
    and ax, 0xf000
    and edx, 0xffff
    push ax // CS
    push edx // EIP
    retf

not_reachable:

error:
    mov ah, 0x01    // Send a character feature.
    mov dx, 0x0000  // The COM port.
    int 0x14
    hlt

// DI: dest address, SI: cluster offset, CX: # of clusters
read_kernel_file:
    pusha
    mov ax, [KERNEL_FIRST_CLUSTER]

read_cluster:
    // Skip clusters.
    sub si, 1
    cmp si, 0
    jge next_cluster

    pusha
    sub ax, 2 // Cluster number *typically* starts with 2.
    shl ax, 3 // assuming that 8 sectors per cluster TODO
    add ax, [FIRST_CLUSTER_SECTOR]
    mov word ptr [DAP_SECTOR], ax
    mov word ptr [DAP_DEST], di
    shr edi, 4
    and di, 0xf000
    mov word ptr [DAP_DEST_SEG], di
    mov word ptr [DAP_NUM], SECTORS_PER_CLUSTER
    popa
    call read_sector
    add di, SECTORS_PER_CLUSTER * 512

next_cluster:
    mov bx, FAT_TABLE
    shl ax, 2
    add bx, ax
    mov edx, [bx] // The next cluster number.

    pusha
    mov di, dx
    lea si, fat_end_marker
    mov cx, 4
    repe cmpsb
    popa
    je 1f

    mov ax, dx

    // Skip clusters.
    cmp si, 0
    jge read_cluster

    loop read_cluster
1:
    popa
    ret

read_sector:
    pusha

    mov ah, 0x42
    mov si, DAP
    mov dl, [DRIVER_NUMBER]
    xchg bx,bx
    int 0x13

    popa
    ret

// Look for the signature in the disk. Returns in SI the offset of signature
// in the buffer.
look_for_signature:
    mov bx, 0x8000
    mov word ptr [DAP_DEST], bx
    mov word ptr [DAP_NUM], 1
    mov bx, 1024   // The maximum sector number.

try_next_sector:
    // Read a sector.
    mov word ptr [DAP_SECTOR], dx
    call read_sector
    mov di, 0x8000

search_sector:
    pusha
    mov si, ax
    repe cmpsb
    popa
    je 2f

    add di, 1
    cmp di, 0x8200
    jne search_sector

    // Try next sector.
    add dx, 1
    sub bx, 1
    jnz try_next_sector

    // The signature is not found.
    mov al, 'N'
    jmp error

2:
    ret

// The partition table. No partitions for now.
.org 0x01be
.space 0x20 // reserved for two partitions

bpb_jump_code:
    .byte 0xeb, 0x58, 0x90 // XXX

fat_table_signature:
    .byte 0xf0, 0xff, 0xff, 0xff

fat_end_marker:
    .byte 0x0ff, 0xff, 0xff, 0xf0

kernel_filename:
    .ascii "KERNEL  ELF"

.org 0x01fe

// The MBR boot signature. "0x55 0xaa" indicates that the partition is bootable.
.byte 0x55
.byte 0xaa