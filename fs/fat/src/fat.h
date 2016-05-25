#ifndef __FAT_FAT_H__
#define __FAT_FAT_H__

#define PACKAGE_NAME "fat"
#include <resea.h>

/* we assume LP64 */
typedef uintmax_t  fat_uintmax_t;
typedef uintmax_t  fat_lba_t;
typedef uintmax_t  fat_size_t;
typedef uintmax_t  fat_offset_t;
typedef uint32_t   fat_cluster_t;

struct fat_disk;

typedef result_t fat_os_read_disk(struct fat_disk *disk, fat_lba_t lba, fat_size_t num,
                                void **data);
typedef result_t fat_os_write_disk(struct fat_disk *disk, fat_lba_t lba, fat_size_t num,
                                 const void *data);

#define FAT_SECTOR_SIZE 512 /* TODO don't change -- code assumes that it is 512 */
#define FAT_CLUSTER_END 0xffffffff
#define FAT_ATTR_READONLY(attr) ((attr) & (1 << 0))
#define FAT_ATTR_HIDDEN(attr)   ((attr) & (1 << 1))
#define FAT_ATTR_SYSTEM(attr)   ((attr) & (1 << 2))
#define FAT_ATTR_DIR(attr)      ((attr) & (1 << 4))

struct fat_bpb {
    uint8_t   jmp[3];
    uint8_t   oem_name[8];
    uint16_t  sector_size;
    uint8_t   sectors_per_cluster;
    uint16_t  reserved_sector_num;
    uint8_t   fat_num;
    uint16_t  root_entries_num;
    uint16_t  total_sector_num16;
    uint8_t   media_id;
    uint16_t  sectors_per_fat16;
    uint16_t  sectors_per_track;
    uint16_t  head_num;
    uint32_t  hidden_sectors;
    uint32_t  total_sector_num32;
    uint32_t  sectors_per_fat32;
    uint16_t  flags;
    uint16_t  fat_version;
    uint32_t  root_entries_cluster;
    uint16_t  fsinfo_sector;
    uint16_t  sectors_per_backup_boot;
    uint8_t   reserved[12];
    uint8_t   drive_number;
    uint8_t   winnt_flags;
    uint8_t   signature;  // it must be 0x28 or 0x29
    uint8_t   volume_id[4];
    uint8_t   volume_label[11];
    uint8_t   fat32_string[8];
    uint8_t   bootcode[420];
    uint8_t   magic[2]; /* 0x55, 0xaa */
} PACKED;

struct fat_entry {
    /*
     *  name[0]
     *
     *    0xe5 / 0x00: unused
     *    0x05: replace with 0xe5
     *
     */
    uint8_t   name[8];
    uint8_t   ext[3];
    uint8_t   attr;
    uint8_t   winnt;
    uint8_t   time_created_tenth;
    uint16_t  time_created;
    uint16_t  date_created;
    uint16_t  date_accessed;
    uint16_t  cluster_begin_high;
    uint16_t  time_modified;
    uint16_t  date_modified;
    uint16_t  cluster_begin_low;
    uint32_t  size;
} PACKED;

struct fat_disk {
    channel_t ch; /* storage_device */
    fat_os_read_disk  *read_disk;
    fat_os_write_disk *write_disk;
    struct fat_bpb *bpb;
    fat_cluster_t fat_entries_cluster;
    fat_lba_t root_dir_lba;
    fat_lba_t data_lba;
    int type; /* 16 or 32 */
};

struct fat_file {
    char name[13]; /* name(8) + dot(3) + ext(1) + NULL(1) == 13 */
    fat_size_t size;
    fat_cluster_t cluster;
    fat_lba_t     lba_offset;
    fat_cluster_t cluster_begin;
    fat_offset_t  offset;
    bool used;
};


struct fat_dir {
    struct fat_entry *entries;
    fat_cluster_t cluster;
    fat_lba_t    lba_offset;
    fat_offset_t index;
};

extern struct fat_disk fat_the_disk;

result_t fat_opendisk(struct fat_disk *disk, channel_t ch,
                    fat_os_read_disk *read_disk,
                    fat_os_write_disk *write_disk);
result_t fat_closedisk(struct fat_disk *disk);

result_t fat_open(struct fat_disk *disk, struct fat_file *file, const char *path,
                size_t path_size);
result_t fat_close(struct fat_disk *disk, struct fat_file *file);
result_t fat_read(struct fat_disk *disk, struct fat_file *file,
                fat_offset_t offset, void *buf, fat_size_t size,
                size_t *r_size);
result_t fat_write(struct fat_disk *disk, struct fat_file *file, const void *buf, fat_size_t size);

result_t fat_opendir(struct fat_disk *disk, struct fat_dir *dir, const char *path,
                   size_t path_size);
result_t fat_closedir(struct fat_disk *disk, struct fat_dir *dir);
result_t fat_readdir(struct fat_disk *disk, struct fat_dir *dir, struct fat_entry *entry);
result_t fat_mkdir(struct fat_disk *disk, const char *path);

#endif

