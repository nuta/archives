#ifndef __ELF_H__
#define __ELF_H__

#define PACKAGE_NAME "elf"

#include <resea.h>

#define STACK_SIZE 0x2000
#define STACK_START_ADDRESS 0x90000000

enum elf_archtype{
  ELF_ARCH_X86_64
};

#define EI_NIDENT   16
#define EI_MAG0     0x7f
#define EI_MAG1     'E'
#define EI_MAG2     'L'
#define EI_MAG3     'F'
#define ELFCLASS64  2
#define EM_X86_64   62
#define PT_LOAD     1
#define PF_R        4
#define PF_W        2
#define PF_X        1

/*
 *  Elf64
 */
typedef uint64_t  Elf64_uintptr_t;
typedef uint64_t  Elf64_Off;

typedef struct{
  uint8_t       e_ident[EI_NIDENT];
  uint16_t      e_type;
  uint16_t      e_machine;
  uint32_t      e_version;
  Elf64_uintptr_t  e_entry;
  Elf64_Off   e_phoff;
  Elf64_Off   e_shoff;
  uint32_t      e_flags;
  uint16_t      e_ehsize;
  uint16_t      e_phentsize;
  uint16_t      e_phnum;
  uint16_t      e_shentsize;
  uint16_t      e_shnum;
  uint16_t      e_shstrndx;
} Elf64_Ehdr;

typedef struct {
  uint32_t     p_type;
  uint32_t     p_flags;
  Elf64_Off  p_offset;
  Elf64_uintptr_t p_vaddr;
  Elf64_uintptr_t p_paddr;
  uint64_t     p_filesz;
  uint64_t     p_memsz;
  uint64_t     p_align;
} Elf64_Phdr;

typedef struct {
   uint32_t     sh_name;
   uint32_t     sh_type;
   uint64_t     sh_flags;
   Elf64_uintptr_t sh_addr;
   Elf64_Off  sh_offset;
   uint64_t     sh_size;
   uint32_t     sh_link;
   uint32_t     sh_info;
   uint64_t     sh_addralign;
   uint64_t     sh_entsize;
} Elf64_Shdr;

/*
 *  functions
 */
result_t elf_validate(void *data, enum elf_archtype archtype);
result_t elf_load_executable(channel_t memory_ch, ident_t group,
                           channel_t fs, channel_t zeroed_pager, ident_t file,
                           uintptr_t *entry, uintptr_t *stack);

#endif
