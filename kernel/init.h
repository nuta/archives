#ifndef __INIT_H__
#define __INIT_H__

#include <kernel/types.h>

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


#define KFS_MAGIC "\x0f\x0bKFS!!" // terminated by null character

struct kfs_header {
    char magic[8];
} PACKED;

struct kfs_file_header {
    const char name[128]; // terminated by null character
    u32_t length;
} PACKED;

struct kfs_dir {
    struct kfs_file_header *current;
};

struct kfs_file {
    const char *name;
    size_t length;
    const void *data;
    struct kfs_file_header *pager_arg;
};

void kernel_init(void);

#endif
