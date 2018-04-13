.intel_syntax noprefix
.code16

#define PT_LOAD 1
#define SECTOR_SIZE 512
#define ELF_MAGIC 0x7f
#define ELF_HEADER_SIZE 64

// Memory Areas.
// Assuming that 0x8000-0x8fff are filled by zero.
#define KERNEL_FIRST_CLUSTER     0x8000 /* word */
#define FIRST_CLUSTER_SECTOR     0x8002 /* word */
#define DRIVER_NUMBER            0x8004 /* byte */
#define SECTORS_PER_CLUSTER      0x8006 /* byte */
#define LOG2_SECTORS_PER_CLUSTER 0x8007 /* byte */

// Disk address Packet
// https://en.wikipedia.org/wiki/INT_13H#INT_13h_AH=42h:_Extended_Read_Sectors_From_Drive
#define DAP          0x8010
#define DAP_SIZE     0x8010 /* byte */
#define DAP_NUM      0x8012 /* word */
#define DAP_DEST     0x8014 /* word */
#define DAP_DEST_SEG 0x8016 /* word */
#define DAP_SECTOR   0x8018 /* quad */

#define LOOK_FOR_SIG_BUF 0x9000
#define FAT_TABLE 0xa000
#define ELF_HEADER 0xf000
#define FAT_TABLE_SECTORS ((ELF_HEADER - FAT_TABLE) / SECTOR_SIZE)

.globl boot
boot:
    cli
    cld
    mov sp, 0x7c00
    xor ax, ax
    mov ds, ax
    mov ss, ax

    // DL register holds the drive index set by BIOS.
    mov [DRIVER_NUMBER], dl

    // Initialize the serial port.
    mov ax, 0x00e3  // baud_rate=9600, no parity,
                    // one stop bit, one character is 8bits
    xor dx, dx      // The COM port.
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

    mov al, [di + 0x0d]
    mov [SECTORS_PER_CLUSTER], al

    // Compute log2(AL). Assuming that AL is a power of 2.
    bsf cx, ax
    mov [LOG2_SECTORS_PER_CLUSTER], cl

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
    mov word ptr [DAP_NUM], FAT_TABLE_SECTORS
    call read_sector

    // Load ELF header.
    mov di, ELF_HEADER
    xor si, si
    mov cx, 1
    call read_kernel_file

    // Verify magic number.
    mov al, 'E'
    cmp byte ptr [di], ELF_MAGIC
    jne error

valid_elf:

    // # of program headers.
    mov dx, [ELF_HEADER + 0x38]
    mov cl, [LOG2_SECTORS_PER_CLUSTER]
    add cl, 9 // assuming that offset is multiple of 512

    // Load segments.
    mov bx, ELF_HEADER + ELF_HEADER_SIZE // the first program header offset

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
    dec dx
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
    xor dx, dx // The COM port.
    int 0x14
    hlt

// DI: dest address, SI: cluster offset, CX: # of clusters
read_kernel_file:
    pusha
    mov ax, [KERNEL_FIRST_CLUSTER]

read_cluster:
    // Skip clusters.
    dec si
    cmp si, 0
    jge next_cluster

    pusha
    sub ax, 2 // Cluster number *typically* starts with 2.
    mov cl, [LOG2_SECTORS_PER_CLUSTER]
    shl ax, cl // Divide by SECTORS_PER_CLUSTER
    add ax, [FIRST_CLUSTER_SECTOR]
    mov word ptr [DAP_SECTOR], ax
    mov word ptr [DAP_DEST], di
    shr edi, 4
    and di, 0xf000
    mov word ptr [DAP_DEST_SEG], di
    mov al, [SECTORS_PER_CLUSTER]
    mov byte ptr [DAP_NUM], al
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
    int 0x13

    popa
    ret

// Look for the signature in the disk. Returns in SI the offset of signature
// in the buffer.
look_for_signature:
    mov bx, LOOK_FOR_SIG_BUF
    mov word ptr [DAP_DEST], bx
    mov word ptr [DAP_NUM], 1
    mov bx, 1024   // The maximum sector number.

try_next_sector:
    // Read a sector.
    mov word ptr [DAP_SECTOR], dx
    call read_sector
    mov di, LOOK_FOR_SIG_BUF

search_sector:
    pusha
    mov si, ax
    repe cmpsb
    popa
    je 2f

    inc di
    cmp di, LOOK_FOR_SIG_BUF + SECTOR_SIZE
    jne search_sector

    // Try next sector.
    inc dx
    dec bx
    jnz try_next_sector

    // The signature is not found.
    mov al, 'N'
    jmp error

2:
    ret

bpb_jump_code:
    .byte 0xeb, 0x58, 0x90 // XXX

fat_table_signature:
    .byte 0xf0, 0xff, 0xff, 0xff

fat_end_marker:
    .byte 0x0ff, 0xff, 0xff, 0xf0

kernel_filename:
    .ascii "KERNEL  ELF"

// The partition table.
.org 0x01be
.space 0x40

// The MBR boot signature.
.byte 0x55
.byte 0xaa