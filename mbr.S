.intel_syntax noprefix
.code16

#define FAT_TABLE 0x9000
#define FAT_TABLE_SIZE_IN_SECTORS ((0xf000 - 0x9000) / 512)
#define ELF_HEADER 0xf000
#define SECTORS_PER_CLUSTER 8

.globl boot
boot:
    // Disable interrupts.
    cli
    cld

    // DL register holds the drive index. It's already set by BIOS.
    mov [drive_number], dl

    // Initialize the stack pointer.
    // According to osdev, 0x0500 - 0x7bff are available.
    mov sp, 0x7c00
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov ss, ax

    // Initialize the serial port.
    // http://webpages.charter.net/danrollins/techhelp/0208.HTM
    mov ah, 0x00    // Initialize serial port feature.
    mov al, 0xe3    // baud_rate=9600, no parity,
                    // one stop bit, one character is 8bits
    mov dx, 0x0000  // The COM port.
    int 0x14

    mov al, '*'
    call printchar

    // Look for the dictory entry.
    lea ax, kernel_filename
    mov dx, 1900      //  The sector number.
    mov cx, 11        // filename (8 bytes) + extension (3 bytes)
    call look_for_signature

    // Get the first cluster number.
    // di points to the the beginning of file name field which
    // is the beginning of a directory entry.
    mov ax, [di + 0x1a] // assuming that higher 16-bits are zero
    mov [kernel_first_cluster], ax

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
    mov [first_cluster_sector], bx

    // Look for the FAT ID and endianess marker. (F0 FF FF FF)
    lea ax, fat_table_signature
    mov dx, 1      //  The sector number.
    mov cx, 4
    call look_for_signature

    // Load the FAT table.
    mov word ptr [dap_sector], dx
    mov word ptr [dap_dest], FAT_TABLE
    mov word ptr [dap_num], FAT_TABLE_SIZE_IN_SECTORS
    call read_sector

    mov al, 'E'
    call printchar

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
    mov edi, [bx + 0x10] // virtual adress
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
    mov ax, [ELF_HEADER + 0x18]
    hlt

    mov bx, ax
    mov al, 'Y'
    jmp error

    jmp ax

not_reachable:
    mov al, 'H'
    jmp error

// DI: dest address, SI: cluster offset, CX: # of clusters
read_kernel_file:
    pusha
    mov ax, [kernel_first_cluster]

read_cluster:
    // Skip clusters.
    sub si, 1
    cmp si, 0
    jge next_cluster

    push ax
    sub ax, 2 // Cluster number *typically* starts with 2.
    shl ax, 3 // assuming that 8 sectors per cluster TODO
    add ax, first_cluster_sector
    mov bx, first_cluster_sector
    mov word ptr [dap_sector], ax
    mov dword ptr [dap_dest], edi
    mov word ptr [dap_num], SECTORS_PER_CLUSTER
    pop ax
    call read_sector
    add di, SECTORS_PER_CLUSTER * 512

next_cluster:
    mov dx, FAT_TABLE
    shl ax, 2

    pusha
    mov di, [dx + ax]
    lea si, fat_end_marker
    repe cmpsb
    je 1f
    popa

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
    lea si, dap
    mov dl, [drive_number]
    int 0x13

    popa
    ret

// Look for the signature in the disk. Returns in SI the offset of signature
// in the buffer.
look_for_signature:
    push bx
    mov bx, 0x8000
    mov word ptr [dap_dest], bx
    mov bx, 1024   // The maximum sector number.

try_next_sector:
    // Read a sector.
    mov word ptr [dap_sector], dx
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

    // JNZ did not perform jump to try_next_sector because the signature is
    // not found.
    mov al, 'N'
    jmp error

2:
    pop bx
    ret

//  Prints a character in the screen.
//
//  Parameters:
//
//    AL - The ASCII character.
//
printchar:
    pusha

    mov ah, 0x01    // Send a character feature.
    mov dx, 0x0000  // The COM port.
    int 0x14

    popa
    ret

error:
    call printchar
    hlt

// Disk drive number.
drive_number:
    .byte 0x00

// Disk address Packet
// https://en.wikipedia.org/wiki/INT_13H#INT_13h_AH=42h:_Extended_Read_Sectors_From_Drive
dap:
    .byte   0x10        // Size of the disk address packet.
    .byte   0x00        // Unused.
dap_num:
    .word   0x0001      // # of sectors to be read.
dap_dest:
    .long   0x00000000  // The destination address.
dap_sector:
    .quad   0x0000000000000000  // The source sector.

kernel_first_cluster:
    .word 0

first_cluster_sector:
    .word 0

bpb_jump_code:
    .byte 0xeb, 0x58, 0x90 // XXX

fat_table_signature:
    .byte 0xf0, 0xff, 0xff, 0xff

fat_end_marker:
    .byte 0x0f, 0xff, 0xff, 0xff

kernel_filename:
    .ascii "KERNEL  ELF"

// Fill 0x00 until 0x01b8 th byte from the beginning of the MBR. From here
// some data about the partition. Refer Wikipedia for details:
//
//   https://en.wikipedia.org/wiki/Master_boot_record#Sector_layout
//
// Here we use "Structure of a classical generic MBR".
// XXX .org 0x01be
.org 0x01fe

// The partition table. No partitions for now.
// XXX .space 0x40

// The MBR boot signature. "0x55 0xaa" indicates that the partition is bootable.
.byte 0x55
.byte 0xaa