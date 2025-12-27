use core::mem::offset_of;

use ftl::error::Error;
use ftl::error::ErrorCode;
use ftl::prelude::*;
use ftl_driver::device::DmaDesc;
use ftl_driver::device::IoSpaceDesc;
use ftl_driver::iospace::IoSpace;
use ftl_driver::oneshot_dma::Direction;
use ftl_driver::oneshot_dma::OneshotDma;
use ftl_utils::endianess::LE;
use virtio::DeviceType;
use virtio::transports::Transport;
use virtio::transports::VirtioMmio;
use virtio::virtqueue::DescBuffer;
use virtio::virtqueue::VIRTQ_SIZE;
use virtio::virtqueue::VirtQueue;

const MAX_PACKET_SIZE: usize = 2048;

#[derive(Error, Debug)]
pub enum Error {
    InitIoSpace(ErrorCode),
    Probe(virtio::transports::mmio::ProbeError),
    OtherDeviceType(DeviceType),
    InitOneshotDma(ftl_driver::oneshot_dma::Error),
    InitVirtq(virtio::virtqueue::Error),
    FillRxQueue(virtio::virtqueue::Error),
    PushAvail(virtio::virtqueue::Error),
    TooLargePacket(usize),
    UnexpectedDescType,
}

#[derive(Debug, Copy, Clone)]
#[repr(C, packed)]
struct PacketHeader {
    flags: u8,
    gso_type: u8,
    hdr_len: LE<u16>,
    gso_size: LE<u16>,
    checksum_start: LE<u16>,
    checksum_offset: LE<u16>,
}

#[repr(C, packed)]
struct DeviceConfig {
    mac: [u8; 6],
    status: LE<u16>,
    max_virtqueue_pairs: LE<u16>,
    mtu: LE<u16>,
    speed: LE<u32>,
    duplex: u8,
    rss_max_key_size: u8,
    rss_max_indirection_table_length: LE<u16>,
    supported_hash_types: LE<u32>,
}

pub struct VirtioNet {
    transport: VirtioMmio,
    rx_queue: VirtQueue,
    tx_queue: VirtQueue,
    payload_dma: OneshotDma<Vec<u8>>,
    mac: [u8; 6],
}

impl VirtioNet {
    pub fn probe(iospace_desc: IoSpaceDesc, dma_desc: DmaDesc) -> Result<Self, Error> {
        let iospace = IoSpace::map(iospace_desc).map_err(Error::InitIoSpace)?;
        let mut virtio_mmio = VirtioMmio::probe(iospace).map_err(Error::Probe)?;

        let device_type = virtio_mmio.device_type();
        if device_type != DeviceType::Net {
            return Err(Error::OtherDeviceType(device_type));
        }

        virtio_mmio.negotiate_features(0);

        let mut rx_queue = virtio_mmio
            .virtqueue(dma_desc, 0)
            .map_err(Error::InitVirtq)?;
        let tx_queue = virtio_mmio
            .virtqueue(dma_desc, 1)
            .map_err(Error::InitVirtq)?;

        let mut payload_dma = OneshotDma::new(dma_desc).map_err(Error::InitOneshotDma)?;

        // Fill RX queue.
        for _ in 0..VIRTQ_SIZE {
            let buffer = vec![0; size_of::<PacketHeader>() + MAX_PACKET_SIZE];
            let len = buffer.len() as u32;
            let baddr = payload_dma
                .map(buffer, Direction::FromDevice)
                .map_err(Error::InitOneshotDma)?;

            let chain = &[DescBuffer::WritableFromDevice { baddr, len }];

            rx_queue.push_avail(chain).map_err(Error::FillRxQueue)?;
        }

        virtio_mmio.initialize();

        // Notify the device that RX buffers are available
        rx_queue.notify(&mut virtio_mmio);

        let mut mac = [0u8; 6];
        virtio_mmio.read_config(offset_of!(DeviceConfig, mac), &mut mac);
        info!(
            "virtio_net: device mac {:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
        );

        Ok(Self {
            transport: virtio_mmio,
            rx_queue,
            tx_queue,
            payload_dma,
            mac,
        })
    }

    pub fn send_packet(&mut self, packet: &[u8]) -> Result<(), Error> {
        if packet.len() > MAX_PACKET_SIZE {
            return Err(Error::TooLargePacket(packet.len()));
        };

        let header = PacketHeader {
            flags: 0,
            gso_type: 0,
            hdr_len: 0.into(),
            gso_size: 0.into(),
            checksum_start: 0.into(),
            checksum_offset: 0.into(),
        };

        // TODO: use zerocopy?
        let header_bytes = unsafe {
            core::slice::from_raw_parts(&raw const header as *const u8, size_of::<PacketHeader>())
        };

        let mut buf: Vec<u8> = Vec::with_capacity(size_of::<PacketHeader>() + packet.len());
        buf.extend_from_slice(header_bytes);
        buf.extend_from_slice(packet);

        // Map the payload. This moves the ownership of the buffer to the device.
        let baddr = self
            .payload_dma
            .map(buf, Direction::ToDevice)
            .map_err(Error::InitOneshotDma)?;

        let chain = vec![
            DescBuffer::ReadOnlyFromDevice {
                baddr,
                len: size_of::<PacketHeader>() as u32,
            },
            DescBuffer::ReadOnlyFromDevice {
                baddr: baddr.offset(size_of::<PacketHeader>()),
                len: packet.len() as u32,
            },
        ];

        self.tx_queue.push_avail(&chain).map_err(Error::PushAvail)?;
        self.tx_queue.notify(&mut self.transport);
        Ok(())
    }

    pub fn handle_irq<F>(&mut self, mut receive: F) -> Result<(), Error>
    where
        F: FnMut(&[u8]),
    {
        let isr_status = self.transport.read_isr_status();
        if isr_status.is_queue_interrupt() {
            // RX queue: forward the packet and push a buffer back to the queue.
            while let Some(chain) = self.rx_queue.pop_used() {
                // Forward the received packet.
                {
                    assert_eq!(chain.descs.len(), 1);
                    let DescBuffer::WritableFromDevice { baddr, .. } = chain.descs[0] else {
                        warn!("unexpected type for RX desc");
                        return Err(Error::UnexpectedDescType);
                    };

                    // Reclaim the ownership of the buffer.
                    let payload_vec = self
                        .payload_dma
                        .unmap(baddr, Direction::FromDevice)
                        .map_err(Error::InitOneshotDma)?;

                    let packet = &payload_vec[size_of::<PacketHeader>()..chain.total_len];
                    receive(packet);
                }

                // Allocate and map a new RX buffer.
                {
                    let buffer = vec![0; size_of::<PacketHeader>() + MAX_PACKET_SIZE];
                    let len = buffer.len() as u32;
                    let baddr = self
                        .payload_dma
                        .map(buffer, Direction::FromDevice)
                        .map_err(Error::InitOneshotDma)?;

                    // Add a new buffer to the RX queue.
                    self.rx_queue
                        .push_avail(&[DescBuffer::WritableFromDevice { baddr, len }])
                        .map_err(Error::FillRxQueue)?;
                    self.rx_queue.notify(&mut self.transport);
                }
            }

            // TX queue: release sent TX buffers.
            while let Some(chain) = self.tx_queue.pop_used() {
                assert_eq!(chain.descs.len(), 2 /* header + payload */);

                // 1st descriptor: virtio-net header (PacketHeader).
                let DescBuffer::ReadOnlyFromDevice {
                    baddr,
                    len: header_len,
                } = chain.descs[0]
                else {
                    warn!("unexpected type for TX header desc");
                    return Err(Error::UnexpectedDescType);
                };

                debug_assert_eq!(header_len as usize, size_of::<PacketHeader>());

                // 2nd descriptor: packet payload.
                let DescBuffer::ReadOnlyFromDevice { .. } = chain.descs[1] else {
                    warn!("unexpected type for TX payload desc");
                    return Err(Error::UnexpectedDescType);
                };

                // Reclaim the ownership of the buffer.
                let payload = self
                    .payload_dma
                    .unmap(baddr, Direction::ToDevice)
                    .map_err(Error::InitOneshotDma)?;

                // Release the payload buffer.
                drop(payload);
            }
        }

        Ok(())
    }

    pub fn mac_address(&self) -> [u8; 6] {
        self.mac
    }
}
