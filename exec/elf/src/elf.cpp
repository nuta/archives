#include "elf.h"
#include <resea.h>
#include <resea/fs.h>
#include <resea/memory.h>
#include "elf.h"


/* Checks whether a valid ELF header for `archtype`. */
result_t elf_validate(void *data, enum elf_archtype archtype) {
    Elf64_Ehdr *ehdr;

    ehdr = (Elf64_Ehdr *) data;

    /* check out the magic number */
    if (ehdr->e_ident[0] != EI_MAG0 || ehdr->e_ident[1] != EI_MAG1 ||
            ehdr->e_ident[2] != EI_MAG2 || ehdr->e_ident[3] != EI_MAG3)
        return E_INVALID;

    switch(archtype) {
    case ELF_ARCH_X86_64:
        if (ehdr->e_ident[4] != ELFCLASS64)
            return E_INVALID;
        if (ehdr->e_machine != EM_X86_64)
            return E_INVALID;
        break;
    }

    return OK;
}


#define PHDR_MAX 4

/* Loads and maps a ELF executable into the virtual memory and returns
   the entry point. */
result_t elf_load_executable(channel_t memory_ch, ident_t group,
                           channel_t fs, channel_t zeroed_pager, ident_t file,
                           uintptr_t *entry, uintptr_t *stack) {
    void *data;
    result_t r;
    size_t size;
    Elf64_Ehdr *ehdr;
    Elf64_Phdr *phdr;
    int i;

    /* load and validate the header */
    resea::interfaces::fs::call_read(fs,
         file, 0, sizeof(Elf64_Ehdr) + (sizeof(Elf64_Phdr) * PHDR_MAX),
         &r, &data, &size);

    if (r != OK)
        return r;

    INFO("verifying the ELF header");
    if (elf_validate(data, ELF_ARCH_X86_64 /* FIXME */) != OK) {
        INFO("invalid ELF header, aborting");
        return E_INVALID;
    }

    ehdr = (Elf64_Ehdr *) data;
    if (ehdr->e_phoff == 0) {
        WARN("tried to load a invalid file as an executable");
        return E_INVALID;
    }

    /* load program headers */
    INFO("reading program headers");
    for (i=0; i < ehdr->e_phnum && i < PHDR_MAX; i++) {
        phdr = (Elf64_Phdr *) ((uintptr_t)  data + ehdr->e_phoff + (ehdr->e_phentsize * i));

        if (phdr->p_type == PT_LOAD) {
            uint8_t flags = 0;
            flags |= (phdr->p_flags & PF_R)? resea::interfaces::memory::MAP_READ  : 0;
            flags |= (phdr->p_flags & PF_W)? resea::interfaces::memory::MAP_WRITE : 0;
            flags |= (phdr->p_flags & PF_X)? resea::interfaces::memory::MAP_EXEC  : 0;

            /* map the area to the virtual memory space */
            INFO("mapping a pager: vaddr=%p, size=%d type=%c%c%c",
                 phdr->p_vaddr, phdr->p_filesz,
                 (phdr->p_flags & PF_R)? 'R' : '-',
                 (phdr->p_flags & PF_W)? 'W' : '-',
                 (phdr->p_flags & PF_X)? 'X' : '-');

            resea::interfaces::memory::call_map(memory_ch,
                 group, phdr->p_vaddr, phdr->p_filesz << 8 | flags,
                 fs, file, phdr->p_offset,
                 &r);
        }
    }

    /* stack */
   INFO("mapping the zeroed_pager for stack: vaddr=%p, size=%d",
        STACK_START_ADDRESS, STACK_SIZE);
    resea::interfaces::memory::call_map(memory_ch,
         group, STACK_START_ADDRESS, STACK_SIZE | resea::interfaces::memory::MAP_READ | resea::interfaces::memory::MAP_WRITE,
         zeroed_pager, 0, 0,
         &r);

    *entry = ehdr->e_entry;
    *stack = STACK_START_ADDRESS;
    return OK;
}
