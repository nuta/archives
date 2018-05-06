#ifndef __ELF_H__
#define __ELF_H__

#include "types.h"
#include "memory.h"

#define ELF_MAGIC "\x7f" "ELF"
#define ELFCLASS64 2
#define EM_X86_64 62
#define PT_LOAD 1
#define PF_R 4
#define PF_W 2
#define PF_X 1

struct elf64_ehdr{
  u8_t  e_ident[16];
  u16_t e_type;
  u16_t e_machine;
  u32_t e_version;
  u64_t e_entry;
  u64_t e_phoff;
  u64_t e_shoff;
  u32_t e_flags;
  u16_t e_ehsize;
  u16_t e_phentsize;
  u16_t e_phnum;
  u16_t e_shentsize;
  u16_t e_shnum;
  u16_t e_shstrndx;
};

struct elf64_phdr {
  u32_t p_type;
  u32_t p_flags;
  u64_t p_offset;
  u64_t p_vaddr;
  u64_t p_paddr;
  u64_t p_filesz;
  u64_t p_memsz;
  u64_t p_align;
};

struct elf64_shdr {
   u32_t sh_name;
   u32_t sh_type;
   u64_t sh_flags;
   u64_t sh_addr;
   u64_t sh_offset;
   u64_t sh_size;
   u32_t sh_link;
   u32_t sh_info;
   u64_t sh_addralign;
   u64_t sh_entsize;
};

void elf_create_process(const void *image, size_t length, pager_t *pager, void *pager_arg);

#endif
