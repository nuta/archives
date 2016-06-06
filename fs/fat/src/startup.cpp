#include "fat.h"
#include <resea.h>
#include <resea/channel.h>
#include <resea/fs.h>
#include <resea/storage_device.h>
#include "fat.h"
using namespace fat;

channel_t storage_device_ch;
struct disk fat::instance;

static result_t read_disk(struct disk *disk, lba_t lba, size_t num,
                        void **data) {
    result_t result;
    size_t size;

    call_storage_device_read(disk->ch,
         lba * FAT_SECTOR_SIZE, num * FAT_SECTOR_SIZE,
         &result, data, &size);

    return result;
}


static result_t write_disk(struct disk *disk, lba_t lba, size_t num,
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
    result_t r;

    INFO("starting");

    storage_device_ch = create_channel();
    call_channel_connect(connect_to_local(1), storage_device_ch,
        INTERFACE(storage_device), &r);

    opendisk(&instance, storage_device_ch, read_disk, write_disk);

    ch = create_channel();
    call_channel_register(connect_to_local(1), ch,
        INTERFACE(fs), &r);
    serve_channel(ch, fat_handler);
}
