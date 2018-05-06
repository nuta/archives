#ifndef __KFS_H__
#define __KFS_H__

#include <kernel/types.h>

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

void kfs_init(void);
void kfs_opendir(struct kfs_dir *dir);
paddr_t kfs_pager(void *arg, uptr_t addr, size_t length);
struct kfs_file *kfs_readdir(struct kfs_dir *dir, struct kfs_file *file);

#endif
