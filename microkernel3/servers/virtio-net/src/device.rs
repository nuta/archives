use core::intrinsics::copy_nonoverlapping;
use core::mem::size_of;
use resea::arch::x64::pmalloc;
use virtio::{Virtio, VirtioRequest, IO_DEVICE_SPECIFIC, VIRTIO_DESC_F_READ_ONLY};

const VIRTIO_NET_RX_QUEUE: u8 = 0;
const VIRTIO_NET_TX_QUEUE: u8 = 1;
const VIRTIO_NET_IO_MAC_ADDR: u16 = IO_DEVICE_SPECIFIC;

#[repr(packed)]
struct VirtioNetHeader {
    gso_type: u8,
    flags: u8,
    header_len: u16,
    gso_len: u16,
    checksum_start: u16,
    checksum_offset: u16,
}

pub struct VirtioNet {
    virtio: Virtio,
    hwaddr: [u8; 6],
}

impl VirtioNet {
    pub fn new(bar0: u16) -> VirtioNet {
        let mut virtio = Virtio::new(bar0);
        virtio.setup();
        let feats = virtio.get_features();
        virtio.set_features(feats);
        virtio.setup_queue(VIRTIO_NET_RX_QUEUE, true);
        virtio.setup_queue(VIRTIO_NET_TX_QUEUE, false);
        virtio.activate();

        let macaddr: [u8; 6] = unsafe {
            [
                virtio.ioport.in8(VIRTIO_NET_IO_MAC_ADDR + 0),
                virtio.ioport.in8(VIRTIO_NET_IO_MAC_ADDR + 1),
                virtio.ioport.in8(VIRTIO_NET_IO_MAC_ADDR + 2),
                virtio.ioport.in8(VIRTIO_NET_IO_MAC_ADDR + 3),
                virtio.ioport.in8(VIRTIO_NET_IO_MAC_ADDR + 4),
                virtio.ioport.in8(VIRTIO_NET_IO_MAC_ADDR + 5),
            ]
        };

        println!(">>> macaddr: {:?}", macaddr);
        VirtioNet {
            virtio: virtio,
            hwaddr: macaddr,
        }
    }

    pub fn get_hwaddr(&self) -> &[u8] {
        &self.hwaddr
    }

    pub fn send(&self, data: &[u8]) -> Result<(), ()> {
        let (header_vaddr, header_paddr) = pmalloc(0, 0, size_of::<VirtioNetHeader>());
        let (data_vaddr, data_paddr) = pmalloc(0, 0, data.len());

        let header = header_vaddr as *mut VirtioNetHeader;
        unsafe {
            copy_nonoverlapping(data.as_ptr(), data_vaddr as *mut u8, data.len());
        }

        unsafe {
            (*header).gso_type = 0;
            (*header).flags = 0;
            (*header).header_len = 0;
            (*header).gso_len = 0;
            (*header).checksum_start = 0;
            (*header).checksum_offset = 0;
        }

        let rs = [
            VirtioRequest {
                data: header_paddr,
                len: size_of::<VirtioNetHeader>(),
                flags: VIRTIO_DESC_F_READ_ONLY,
            },
            VirtioRequest {
                data: data_paddr,
                len: data.len(),
                flags: VIRTIO_DESC_F_READ_ONLY,
            },
        ];

        self.virtio.enqueue_request(VIRTIO_NET_TX_QUEUE, &rs);
        self.virtio.kick_request(VIRTIO_NET_TX_QUEUE);

        // TODO: free pmalloc'ed areas
        Ok(())
    }

    pub fn recv(&self) -> Option<&[u8]> {
        let data = if let Some(data) = self.virtio.dequeue_response(VIRTIO_NET_RX_QUEUE) {
            let header_len = 10; /* sizeof(struct virtio_net_hdr) */
            Some(&data[header_len..])
        } else {
            None
        };

        self.virtio.ack_interrupt();
        data
    }
}
