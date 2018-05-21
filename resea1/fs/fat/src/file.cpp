#include "fat.h"
#include "file.h"
using namespace fat;

#define FILES_MAX 128
struct file files[FILES_MAX];

ident_t fat_alloc_file_id(void) {
    ident_t i;
    for (i=1; i < FILES_MAX; i++) {
        // TODO: add lock
        if (!files[i].used) {
            files[i].used = true;
            return i;
        }
    }

    WARN("failed to allocate a file id");
    return 0;
}


struct file *fat_get_file_by_id(ident_t id) {

    return &files[id];
}


void fat_free_file_by_id(ident_t id) {

    files[id].used = false;
}
