use virtio::{Virtio, VirtioRequest, VIRTIO_DESC_F_READ_ONLY, VIRTIO_DESC_F_WRITE_ONLY};
use resea::arch::x64::{pmalloc};
use resea::mem::size_of;

pub const SECTOR_SIZE: usize = 512;
const VIRTIO_BLK_RQUEUE: u8 = 0;
const VIRTIO_BLK_WRITE: u32 = 1;
const VIRTIO_BLK_READ: u32 = 0;

#[repr(packed)]
struct VirtioBlkHeader {
  type_: u32,     /* VIRTIO_READ or VIRTIO_BLK_WRITE */
  priority: u32, /* priority (0 is lowest)  */
  sector: u64   /* where to read/write */
}

struct VirtioBlkStatus {
    status: u8
}

pub struct VirtioBlk {
    virtio: Virtio
}

impl VirtioBlk {
    pub fn new(bar0: u16) -> VirtioBlk {
        let mut virtio = Virtio::new(bar0);
        virtio.setup();
        let feats = virtio.get_features();
        virtio.set_features(feats);
        virtio.setup_queue(VIRTIO_BLK_RQUEUE);
        virtio.activate();

        VirtioBlk {
             virtio: virtio
        }
    }

    pub fn read(&self, sector: u64, len: usize) -> *const u8 {
        let (header_vaddr, header_paddr) = pmalloc(0, 0, size_of::<VirtioBlkHeader>());
        let (data_vaddr, data_paddr) = pmalloc(0, 0, len);
        let (status_vaddr, status_paddr) = pmalloc(0, 0, size_of::<VirtioBlkStatus>());

        let header = header_vaddr as *mut VirtioBlkHeader;
        let status = status_vaddr as *mut VirtioBlkStatus;

        unsafe {
            (*header).type_  = VIRTIO_BLK_READ;
            (*header).sector = sector;
        }

        let rs = [
            VirtioRequest {
                data:  header_paddr,
                len:   size_of::<VirtioBlkHeader>(),
                flags: VIRTIO_DESC_F_READ_ONLY,
            },
            VirtioRequest {
                data:  data_paddr,
                len:   len,
                flags: VIRTIO_DESC_F_WRITE_ONLY,
            },
            VirtioRequest {
                data:  status_paddr,
                len:   size_of::<VirtioBlkStatus>(),
                flags: VIRTIO_DESC_F_WRITE_ONLY,
            }
        ];

        self.virtio.request(VIRTIO_BLK_RQUEUE, &rs);

        // TODO: free pmalloc'ed areas
        data_vaddr as *const u8
    }
}
