#include <resea.h>

void fat_fs_open(channel_t __ch, uchar_t* a0, size_t a1, fs_filemode_t a2);
void fat_fs_close(channel_t __ch, ident_t a0);
void fat_fs_read(channel_t __ch, ident_t a0, offset_t a1, size_t a2);
void fat_fs_write(channel_t __ch, ident_t a0, offset_t a1, void * a2, size_t a3);
void fat_pager_fill(channel_t __ch, ident_t a0, offset_t a1, size_t a2);
