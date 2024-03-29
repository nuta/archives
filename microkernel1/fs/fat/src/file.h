#ifndef __FAT_FILE_H__
#define __FAT_FILE_H__

#include "fat.h"

ident_t fat_alloc_file_id(void);
struct fat::file *fat_get_file_by_id(ident_t id);
void fat_free_file_by_id(ident_t id);

#endif
