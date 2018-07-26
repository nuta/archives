#include "memory.h"
#include "printk.h"
#include "process.h"
#include "thread.h"
#include "server.h"
#include "string.h"
#include "init.h"
#include "kfs.h"

extern char __kfs[];
void kfs_container(void) {
    INLINE_ASM(".align 8                         \n");
    INLINE_ASM(".globl __kfs                     \n");
    INLINE_ASM("__kfs:                           \n");
    INLINE_ASM(".incbin \"build/kernel/kfs.bin\" \n");
}


static void kfs_opendir(struct kfs_dir *dir) {
    dir->current = (struct kfs_file_header *) ((uptr_t) &__kfs + sizeof(struct kfs_header));
}


struct kfs_file *kfs_readdir(struct kfs_dir *dir, struct kfs_file *file) {
    if (dir->current->name[0] == '\0') {
        return NULL;
    }

    file->name = dir->current->name;
    file->length = dir->current->length;
    file->data = (void *) ((uptr_t) dir->current + sizeof(struct kfs_file_header));
    file->pager_arg = dir->current;

    size_t len = sizeof(struct kfs_file_header) + dir->current->length;
    dir->current = (struct kfs_file_header *) ((uptr_t) dir->current + len);
    return file;
}


static paddr_t kfs_pager(void *arg, off_t offset, size_t length) {
    struct kfs_file_header *header = arg;
    void *data = (void *) ((uptr_t) arg + sizeof(struct kfs_file_header) + offset);
    paddr_t paddr = alloc_pages(GET_PAGE_NUM(length), KMALLOC_NORMAL);
    void *ptr = from_paddr(paddr);

    // FIXME: This may perform memcpy beyond the file data if offset > 0.
    memcpy(ptr, data, min(length, header->length));
    return paddr;
}


void kfs_init(void) {
    struct kfs_header *header = (struct kfs_header *) &__kfs;
    if (strcmp(KFS_MAGIC, (const char *) &header->magic) != 0) {
        PANIC("kfs: invalid magic");
    }
}


static void elf_create_process(const void *image, UNUSED size_t length, pager_t *pager, void *pager_arg) {
    struct elf64_ehdr *ehdr = (struct elf64_ehdr *) image;

    /* check out the magic number */
    if (strncmp((const char *) &ehdr->e_ident, ELF_MAGIC, 4) != 0) {
        WARN("%s: invalid elf magic, skipping...", image);
        return;
    }

    if (ehdr->e_ident[4] != ELFCLASS64 || ehdr->e_machine != EM_X86_64) {
        WARN("%s: unsupported type, skipping...", image);
        return;
    }

    struct process *process = process_create();
    channel_connect(kernel_channel, process);
    DEBUG("elf: created process #%d", process->pid);

    /* Load program headers. */
    for (int i = 0; i < ehdr->e_phnum; i++) {
        struct elf64_phdr * phdr = (struct elf64_phdr *) ((uptr_t) image + ehdr->e_phoff + (ehdr->e_phentsize * i));
        if (phdr->p_type == PT_LOAD) {
            int flags = PAGE_USER;
            flags |= (phdr->p_flags & PF_W)? PAGE_WRITABLE : 0;

            if (phdr->p_filesz == 0) {
                // TODO: Ensure that this is a .bss section
                add_vmarea(&process->vms, phdr->p_vaddr, phdr->p_offset,
                    phdr->p_memsz, flags, zeroed_pager, NULL);
            } else {
                add_vmarea(&process->vms, phdr->p_vaddr, phdr->p_offset,
                    phdr->p_filesz, flags, pager, pager_arg);
            }
        }
    }

    struct thread *thread = thread_create(process, ehdr->e_entry, 0);
    thread_resume(thread);
}


void launch_servers(void) {
    struct kfs_dir dir;
    struct kfs_file file;
    kfs_opendir(&dir);
    while (kfs_readdir(&dir, &file) != NULL) {
        if (!strncmp("/servers/", file.name, 9)) {
            DEBUG("kernel: starting %s", file.name);
            elf_create_process(file.data, file.length, kfs_pager, file.pager_arg);
        }
    }
}
