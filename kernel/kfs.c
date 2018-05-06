#include "memory.h"
#include "kfs.h"
#include "string.h"

extern char __kfs[];


void kfs_opendir(struct kfs_dir *dir) {
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


paddr_t kfs_pager(void *arg, off_t offset, size_t length) {
    struct kfs_file_header *header = arg;
    void *data = (void *) ((uptr_t) arg + sizeof(struct kfs_file_header) + offset);
    paddr_t paddr = alloc_pages(length, KMALLOC_NORMAL);
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


void kfs_container(void) {
    INLINE_ASM(".align 8                         \n");
    INLINE_ASM(".globl __kfs                     \n");
    INLINE_ASM("__kfs:                           \n");
    INLINE_ASM(".incbin \"build/kernel/kfs.bin\" \n");
}
