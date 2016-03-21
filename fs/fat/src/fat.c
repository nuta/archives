#include <string.h>
#include <ctype.h>
#include "fat.h"

#define ROOT_DIR_CLUSTER 0


static int compare_filename(struct fat_entry *e, const char *name, const char *ext) {

    return (!strncmp((char *) e->name, (char *) name, strlen(name)) &&
            !strncmp((char *) e->ext,  (char *) ext,  strlen(ext)));
}


static fat_cluster_t get_cluster_from_entry(struct fat_entry *e) {

    return ((e->cluster_begin_high << 16) | e->cluster_begin_low);
}


static fat_lba_t cluster_to_lba(struct fat_disk *disk, fat_cluster_t cluster) {

    if (cluster == ROOT_DIR_CLUSTER) {
        // in FAT16, root directory entries is stored in front of data clusters
        // XXX: fix this
        return disk->root_dir_lba;
    } else {
        // data cluster # starts with 2
        return (((cluster - 2) * disk->bpb->sectors_per_cluster) + disk->data_lba);
    }
}


/* Returns the next cluster if it exists or FAT_CLUSTER_END if it does not exist. */
static fat_cluster_t get_next_cluster(struct fat_disk *disk, fat_cluster_t from) {
    fat_cluster_t next;
    fat_lba_t lba_offset; /* offset from the beginning of FAT entries */
    int index; /* index in a sector */
    int r;

    if (from == ROOT_DIR_CLUSTER) {
        WARN("multiple cluster root entries are not supported");
        return 0;
    }

    if (disk->type == 32) {
        uint32_t *entries;
        lba_offset = (from * sizeof(uint32_t)) / disk->bpb->sector_size;
        index      = (from * sizeof(uint32_t)) % disk->bpb->sector_size;
    
        if ((r = disk->read_disk(disk,
                                 cluster_to_lba(disk, disk->fat_entries_cluster) +
                                 lba_offset,
                                 1, (void **) &entries)) != OK)
            return r;
    
        next = entries[index] & 0x0fffffff;
        return ((next < 0xffffff8)? next : FAT_CLUSTER_END);
    } else if (disk->type == 16) {
        uint16_t *entries;
        lba_offset = (from * sizeof(uint16_t)) / disk->bpb->sector_size;
        index      = (from * sizeof(uint16_t)) % disk->bpb->sector_size;
    
        if ((r = disk->read_disk(disk,
                                 cluster_to_lba(disk, disk->fat_entries_cluster) +
                                 lba_offset,
                                 1, (void **) &entries)) != OK)
            return r;
    
        next = entries[index] & 0x0ffff;
        return ((next < 0xfff8)? next : FAT_CLUSTER_END);
    } else {
        WARN("not supported FAT type: %d", disk->type);
        return 0;
    }
}


void fat_opendir_by_entry(struct fat_disk *disk, struct fat_dir *dir, struct fat_entry *e) {

    dir->cluster    = get_cluster_from_entry(e);
    dir->index      = 0;
    dir->lba_offset = 0;
}


/*
 *  separate by '/' and copy the current directory level's filename to
 *  `name` and `ext`
 */
static size_t get_next_filename(char **path, size_t path_size, char *name, char *ext) {
  int i;

    INFO("path_size=%d, path='%s'", path_size, *path);
    for (i=0; i < 8 && path_size > 0; i++) {
        if (**path == '/') {
            break;
        }else if (**path == '.') {
            *path += 1;
            break;
        }

        name[i] = toupper(**path);
        *path += 1;
        path_size--;
    }
    name[i] = '\0';

    for (i=0; i < 3 && path_size > 0; i++) {
        if (**path == '/') {
            *path += 1;
            break;
        }

        ext[i] = toupper(**path);
        *path += 1;
        path_size--;
    }
    
    ext[i] = '\0';
    return path_size;
}

static result_t lookup(struct fat_disk *disk, const char *_path, size_t path_size,
                     struct fat_entry *entry) {
    char name[9];
    char ext[4];
    struct fat_dir dir;
    char *path = (char *) _path;

    fat_opendir(disk, &dir, "/", 1);

    /* for each directory level */
    while(1) {
        path_size = get_next_filename(&path, path_size, (char *) &name,
                                      (char *) &ext);

        /* check each file */
        while (1) {
            if (fat_readdir(disk, &dir, entry) != OK) {
                DEBUG("lookup: file not found");
                return E_UNEXPECTED;
            }

            if (compare_filename(entry, (const char *) &name, (const char *) &ext)) {
                if (*path == '\0') {
                    DEBUG("lookup: file found");
                    return OK;
                } else {
                    /* go to the next directory level */
                    fat_opendir_by_entry(disk, &dir, entry);
                    break;
                }
            }
        }
    }
}


result_t fat_opendisk(struct fat_disk *disk, channel_t ch,
                    fat_os_read_disk *read_disk,
                    fat_os_write_disk *write_disk){
    int r;

    disk->ch          = ch;
    disk->read_disk   = read_disk;
    disk->write_disk  = write_disk;

    // read the first sector (BPB)
    if ((r = disk->read_disk(disk, 0, 1, (void **) &disk->bpb)) != OK)
        return r;

    if (!disk->bpb->sector_size)
        return E_INVALID;

    // get the number of clusters
    size_t sectors, data_clusters, fat_size, reserved;
    size_t fat_num, sectors_per_cluster, root_size;
    reserved = disk->bpb->reserved_sector_num;
    fat_num  = disk->bpb->fat_num;
    sectors_per_cluster = disk->bpb->sectors_per_cluster;
    root_size = ((disk->bpb->root_entries_num * 32) + (disk->bpb->sector_size - 1)) / disk->bpb->sector_size;

    if (disk->bpb->sectors_per_fat16 == 0) {
      fat_size = disk->bpb->sectors_per_fat32;
    } else {
      fat_size = disk->bpb->sectors_per_fat16;
    }

    if (disk->bpb->total_sector_num16 == 0) {
      sectors  = disk->bpb->total_sector_num32;
    } else {
      sectors = disk->bpb->total_sector_num16;
    }

    data_clusters = (sectors - (reserved + (fat_num * fat_size) + root_size)) /
                    sectors_per_cluster;

    // verify the disk is FAT32
    if (data_clusters < 4085) {
        WARN("FAT12 is not supported");
        return E_NOTSUPPORTED;
    } else if (data_clusters < 65525) {
        WARN("FAT16 is not supported");
        return E_NOTSUPPORTED;
    } else {
        disk->type = 32;
    }

    disk->fat_entries_cluster  = reserved;
    disk->root_dir_lba         = reserved + fat_size * fat_num;
    disk->data_lba             = disk->root_dir_lba + root_size;

    DEBUG("FAT type:               %d", disk->type);
    DEBUG("sizeof(fat_entry):      %u", sizeof(struct fat_entry));
    DEBUG("sizeof(fat_bpb):        %u", sizeof(struct fat_bpb));
    DEBUG("bpb->oem_name:          %s", (char *) &disk->bpb->oem_name);
    DEBUG("# of reserved sectors:  %d",  reserved);
    DEBUG("# of FAT:               %d",  fat_num);
    DEBUG("# of sectors:           %#x", sectors);
    DEBUG("# of clusters (data):   %#x", data_clusters);
    DEBUG("# of root entries:      %d",  disk->bpb->root_entries_num);
    DEBUG("bytes per sector:       %d",  disk->bpb->sector_size);
    DEBUG("sectors per FAT:        %#x", fat_size);
    DEBUG("sectors per cluster:    %d",  sectors_per_cluster);
    DEBUG("fat_entries_cluster:    %#x", disk->fat_entries_cluster);
    DEBUG("root directory LBA:     %#x", disk->root_dir_lba);
    DEBUG("data cluster LBA:       %#x", disk->data_lba);
    return OK;
}


result_t fat_closedisk(struct fat_disk *disk) {

    // nothing to do
    return OK;
}


result_t fat_open(struct fat_disk *disk, struct fat_file *file, const char *path,
                size_t path_size) {
    int r;
    struct fat_entry e;

    if ((r = lookup(disk, path, path_size, &e)) != OK) {
        return r;
    }

    file->cluster_begin = get_cluster_from_entry(&e);
    file->cluster       = file->cluster_begin;
    file->lba_offset    = 0;
    return OK;
}


result_t fat_close(struct fat_disk *disk, struct fat_file *file) {

    // nothing to do
    return OK;
}


result_t fat_seek(struct fat_disk *disk, struct fat_file *file, fat_offset_t offset) {
    size_t nth_cluster = offset / (disk->bpb->sectors_per_cluster * FAT_SECTOR_SIZE);

    for (int i=0; i < nth_cluster; i++) {
        file->cluster = get_next_cluster(disk, file->cluster);
    }

    file->lba_offset  = offset % (disk->bpb->sectors_per_cluster * FAT_SECTOR_SIZE);
    file->offset      = offset % FAT_SECTOR_SIZE;

    return OK;
}


result_t fat_read(struct fat_disk *disk, struct fat_file *file,
                fat_offset_t offset, void *buf, fat_size_t size,
                size_t *r_size) {
    int r;
    uint8_t *_buf;
    fat_lba_t lba;
    fat_size_t remain = size;

    fat_seek(disk, file, offset);

    while(remain > 0) {
        lba = cluster_to_lba(disk, file->cluster) + file->lba_offset;
        r = disk->read_disk(disk, lba, 1, (void **) &_buf);

        if (r != OK)
            return r;

        memcpy(buf, _buf + file->offset,
               (remain > (FAT_SECTOR_SIZE - file->offset))? (FAT_SECTOR_SIZE - file->offset) : remain);

        if (file->lba_offset >= disk->bpb->sectors_per_cluster) {
            file->cluster    = get_next_cluster(disk, file->cluster);
            file->lba_offset = 0;
        } else {
            file->lba_offset++;
        }

        if (remain < FAT_SECTOR_SIZE)
            break;

        remain -= FAT_SECTOR_SIZE - file->offset;
        buf    += FAT_SECTOR_SIZE;
        file->offset = 0;
    }

    *r_size = size; // TODO
    return OK;
}


result_t fat_write(struct fat_disk *disk, struct fat_file *file, const void *buf, fat_size_t size) {

    // TODO
    return E_UNEXPECTED;
}


result_t fat_opendir(struct fat_disk *disk, struct fat_dir *dir, const char *path,
                   size_t path_size) {
    int r;
    struct fat_entry e;


    if (!strcmp(path, "/")) {
        dir->cluster = ROOT_DIR_CLUSTER;
    } else {
        if ((r = lookup(disk, path, path_size, &e)) != OK) {
            return r;
        }

        dir->cluster = get_cluster_from_entry(&e);
    }

    dir->lba_offset = 0;
    dir->index      = 0;
    return OK;
}


result_t fat_closedir(struct fat_disk *disk, struct fat_dir *dir) {

    /* nothing to do */
    return OK;
}


result_t fat_readdir(struct fat_disk *disk, struct fat_dir *dir, struct fat_entry *entry) {
    /* TODO: cache entries */
    struct fat_entry *entries;
    int r;

    if (dir->cluster == FAT_CLUSTER_END) {
        return E_UNEXPECTED;
    }

    if ((r = disk->read_disk(disk,
                             cluster_to_lba(disk, dir->cluster) + dir->lba_offset,
                             1, (void *) &entries)) != OK)
        return r;

    if (dir->lba_offset >= disk->bpb->sectors_per_cluster) {
        dir->cluster    = get_next_cluster(disk, dir->cluster);
        dir->lba_offset = 0;
    }

    if (disk->bpb->sector_size / sizeof(struct fat_entry) <= dir->index) {
        dir->lba_offset++;
    }

    memcpy(entry, &entries[dir->index], sizeof(struct fat_entry));

    if (entry->name[0] == 0x00) {
        return E_UNEXPECTED;
    }

    dir->index++;

    DEBUG("readdir: name=\"%s\" %s",
          (char *) &entry->name,
          (FAT_ATTR_DIR(entry->attr))? "(DIR)" : "");
    return OK;
}


result_t fat_mkdir(struct fat_disk *disk, const char *path) {

    return E_UNEXPECTED; // TODO
}
