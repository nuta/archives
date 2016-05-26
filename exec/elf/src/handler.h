#include "elf.h"
#include <resea.h>

void elf_exec_create(
    channel_t __ch
    , uchar_t* name
    , size_t name_size
    , channel_t fs
    , ident_t file
    , ident_t group
);
