use core::option::{Option};
use core::cmp::{min};
use core::mem::{size_of, transmute};
use resea::vec::{Vec};
use resea::arch::{ErrorCode};
use resea::server::{ServerResult};
use resea::interfaces::blk_device::{BlkDevice};


pub struct FatFile {
}

#[repr(C)]
#[derive(Clone)]
pub struct FatEntry {
    name: [u8; 8],
    ext: [u8; 3],
    attr: u8,
    ntres: u8,
    time_created_ms: u8,
    time_created: u16,
    date_created: u16,
    date_last_accessed: u16,
    first_cluster_high: u16,
    time_modified: u16,
    date_modified: u16,
    first_cluster_low: u16,
    size: u32,
}

impl FatEntry {
    pub fn cluster(&self) -> Cluster {
        ((self.first_cluster_high as Cluster) << 32) | self.first_cluster_low as Cluster
    }
}

type Cluster = u32;

struct Bpb {
    cluster_size: usize, /* sector_size * sectors_per_cluster */
    root_dir_cluster: Cluster,
    sector_size: usize,
    sectors_per_cluster: usize,
    cluster_start: usize,
    fat_table_sector: usize,
}

fn parse_mbr(mbr: &[u8]) -> Bpb {
    Bpb {
        cluster_size: 0, /* TODO: */
        root_dir_cluster: 0, /* TODO: */
        sector_size: 0,
        sectors_per_cluster: 0,
        cluster_start: 0,
        fat_table_sector: 0,
    }
}

pub struct Fat {
    device: BlkDevice,
    part_begin: u64,
    bpb: Bpb,
}

impl Fat {
    pub fn new(device: BlkDevice, part_begin: u64) -> Fat {
        let bpb = {
            let mbr = device.read(part_begin, 512).unwrap();
            parse_mbr(mbr)
        };

        Fat {
            bpb: bpb,
            device: device,
            part_begin: part_begin,
        }
    }

    pub fn look_for_file(&self, path: &str) -> Option<FatFile> {
        for frag in path.split("/") {
            let mut s = frag.split(".");
            let name = s.nth(0).unwrap_or("");
            let ext = s.nth(1).unwrap_or("");

            let mut current = self.bpb.root_dir_cluster;
            let mut buf = Vec::new();
            while let Some(next) = self.read_cluster(&mut buf, current) {
                for i in 0..(self.bpb.cluster_size / size_of::<FatEntry>()) {
                    // FIXME:
                    let entries = unsafe { transmute::<&[u8], &[FatEntry]>(buf.as_slice()) };
                    let entry = entries[i].clone();

                    if entry.name[0] == 0x00 {
                        // End of entries.
                        return None;
                    }

                    // TODO: match name/ext
                    if false {
                        current = entry.cluster();
                        break;
                    }
                }

                current = next;
            }
        }

        None
    }

    pub fn read_file(&self, buf: &mut Vec<u8>, cluster: Cluster, offset: usize, len: usize) -> ServerResult<usize> {
        let mut current = cluster;
        let mut data = Vec::new();
        let mut total_len = 0;
        let mut remaining = len;
        let mut off = offset;
        while let Some(next) = self.read_cluster(&mut data, current) {
            if off < self.bpb.cluster_size {
                let read_len = min(self.bpb.cluster_size - off, len);
                buf.extend_from_slice(&data[off..(off + read_len)]);
                off = 0;
                remaining -= read_len;
                total_len += read_len;

                if remaining == 0 {
                    return Ok(total_len);
                }
            }

            off -= self.bpb.cluster_size;
            current = next;
        }

        Err(ErrorCode::NotImplemented)
    }

    fn read_cluster(&self, buf: &mut Vec<u8>, cluster: Cluster) -> Option<Cluster> {
        if cluster == 0xffffff00 {
            return None;
        }

        let cluster_offset = (self.bpb.cluster_start + cluster as usize * self.bpb.sectors_per_cluster)
            * self.bpb.sector_size;

        let data = self.device.read(cluster_offset as u64, self.bpb.cluster_size).unwrap();

        let entry_offset = (size_of::<Cluster>() * cluster as usize) % self.bpb.sector_size;
        let fat_offset = self.bpb.fat_table_sector * self.bpb.sector_size + entry_offset % self.bpb.sector_size;
        let fat_data = self.device.read(fat_offset as u64, self.bpb.sector_size).unwrap();
        let table = unsafe { transmute::<&[u8], &[Cluster]>(fat_data) };
        let next = table[entry_offset];
        *buf = data.to_vec();

        Some(next)
    }
}
