#include <resea.h>
#include <resea/fs.h>
#include <resea/storage_device.h>
#include "fat.h"


channel_t storage_device_ch;
struct fat_disk fat_the_disk; // XXX

static result_t read_disk(struct fat_disk *disk, fat_lba_t lba, fat_size_t num,
                        void **data) {
    result_t result;
    size_t size;

    call_storage_device_read(disk->ch,
         lba * FAT_SECTOR_SIZE, num * FAT_SECTOR_SIZE,
         &result, data, &size);

    return result;
}


static result_t write_disk(struct fat_disk *disk, fat_lba_t lba, fat_size_t num,
                         const void *data) {
    result_t result;

    call_storage_device_write(disk->ch,
         lba * FAT_SECTOR_SIZE, (void *) data, num * FAT_SECTOR_SIZE,
         &result);

    return result;
}


handler_t fat_handler;

void fat_startup(void) {
    channel_t ch;

    INFO("starting");
    sys_open(&storage_device_ch);
    connect_channel(storage_device_ch, INTERFACE(storage_device));
    fat_opendisk(&fat_the_disk, storage_device_ch, read_disk, write_disk);

    sys_open(&ch);
    register_channel(ch, INTERFACE(fs));
    serve_channel(ch, fat_handler);
}
