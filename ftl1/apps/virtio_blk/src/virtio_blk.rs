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
use virtio::virtqueue::VirtQueue;

#[derive(Error, Debug)]
pub enum Error {
    InitIoSpace(ErrorCode),
    Probe(virtio::transports::mmio::ProbeError),
    OtherDeviceType(DeviceType),
    InitOneshotDma(ftl_driver::oneshot_dma::Error),
    InitVirtq(virtio::virtqueue::Error),
    PushAvail(virtio::virtqueue::Error),
}

const T_IN: u32 = 0;
const T_OUT: u32 = 1;

enum ReadWrite<'a> {
    Write { data: &'a [u8] },
    Read { len: usize },
}

#[repr(C, packed)]
struct RequestHeader {
    request_type: LE<u32>,
    reserved: LE<u32>,
    sector: LE<u64>,
}

#[repr(C, packed)]
struct ResponseFooter {
    status: u8,
}

const BLK_SIZE: usize = 512;

#[repr(C, packed)]
struct DeviceConfig {
    /// The disk capacity in # of sectors.
    capacity: LE<u64>,
}

pub struct VirtioBlk {
    transport: VirtioMmio,
    requestq: VirtQueue,
    payload_dma: OneshotDma<Vec<u8>>,
}

impl VirtioBlk {
    pub fn probe(iospace_desc: IoSpaceDesc, dma_desc: DmaDesc) -> Result<Self, Error> {
        let iospace = IoSpace::map(iospace_desc).map_err(Error::InitIoSpace)?;
        let virtio_mmio = VirtioMmio::probe(iospace).map_err(Error::Probe)?;

        let device_type = virtio_mmio.device_type();
        if device_type != DeviceType::Blk {
            return Err(Error::OtherDeviceType(device_type));
        }

        virtio_mmio.negotiate_features(0);

        let request_queue = virtio_mmio
            .virtqueue(dma_desc, 0)
            .map_err(Error::InitVirtq)?;

        let payload_dma = OneshotDma::new(dma_desc).map_err(Error::InitOneshotDma)?;

        virtio_mmio.initialize();

        Ok(Self {
            transport: virtio_mmio,
            requestq: request_queue,
            payload_dma,
        })
    }

    pub fn write(&mut self, sector: u64, data: &[u8]) -> Result<(), Error> {
        self.read_write(sector, ReadWrite::Write { data })
    }

    #[allow(dead_code)]
    pub fn read(&mut self, sector: u64, len: usize) -> Result<(), Error> {
        self.read_write(sector, ReadWrite::Read { len })
    }

    fn read_write(&mut self, sector: u64, rw: ReadWrite) -> Result<(), Error> {
        let (request_type, buffer_len) = match rw {
            ReadWrite::Write { data } => (T_OUT, data.len()),
            ReadWrite::Read { len } => (T_IN, len),
        };

        let header = RequestHeader {
            request_type: request_type.into(),
            reserved: 0.into(),
            sector: sector.into(),
        };
        let footer = ResponseFooter { status: 0 };

        let header_bytes = unsafe {
            core::slice::from_raw_parts(&raw const header as *const u8, size_of::<RequestHeader>())
        };
        let footer_bytes = unsafe {
            core::slice::from_raw_parts(&raw const footer as *const u8, size_of::<ResponseFooter>())
        };

        let buf_cap = size_of::<RequestHeader>() + buffer_len + size_of::<ResponseFooter>();
        let mut buf: Vec<u8> = Vec::with_capacity(buf_cap);
        buf.extend_from_slice(header_bytes);
        match rw {
            ReadWrite::Write { data } => {
                buf.extend_from_slice(data);
            }
            ReadWrite::Read { len } => {
                buf.resize(buf.len() + len, 0);
            }
        }
        buf.extend_from_slice(footer_bytes);

        let baddr = self
            .payload_dma
            .map(buf, Direction::ToDevice) // FIXME: ToAndFromDevice
            .map_err(Error::InitOneshotDma)?;

        let buffer_baddr = baddr.offset(size_of::<RequestHeader>());
        let buffer_desc = match rw {
            ReadWrite::Write { .. } => {
                DescBuffer::ReadOnlyFromDevice {
                    baddr: buffer_baddr,
                    len: buffer_len as u32,
                }
            }
            ReadWrite::Read { .. } => {
                DescBuffer::WritableFromDevice {
                    baddr: buffer_baddr,
                    len: buffer_len as u32,
                }
            }
        };

        let chain = vec![
            DescBuffer::ReadOnlyFromDevice {
                baddr,
                len: size_of::<RequestHeader>() as u32,
            },
            buffer_desc,
            DescBuffer::WritableFromDevice {
                baddr: baddr.offset(size_of::<RequestHeader>() + buffer_len),
                len: size_of::<ResponseFooter>() as u32,
            },
        ];

        self.requestq.push_avail(&chain).map_err(Error::PushAvail)?;
        self.requestq.notify(&mut self.transport);
        Ok(())
    }

    pub fn handle_irq(&mut self) -> Result<(), Error> {
        let isr_status = self.transport.read_isr_status();
        if isr_status.is_queue_interrupt() {
            while let Some(chain) = self.requestq.pop_used() {
                assert_eq!(chain.descs.len(), 3);

                let baddr = match chain.descs[0] {
                    DescBuffer::ReadOnlyFromDevice { baddr, .. } => baddr,
                    DescBuffer::WritableFromDevice { .. } => {
                        warn!("expected read-only buffer for request header");
                        continue;
                    }
                };

                let _buf = self
                    .payload_dma
                    .unmap(baddr, Direction::FromDevice)
                    .map_err(Error::InitOneshotDma)?;

                trace!("finished an disk I/O request: {:?}", baddr);
            }
        }

        Ok(())
    }

    pub fn disk_size(&self) -> u128 {
        let mut capacity = [0u8; size_of::<LE<u64>>()];
        self.transport
            .read_config(offset_of!(DeviceConfig, capacity), &mut capacity);
        let capacity = u64::from_le_bytes(capacity);

        capacity as u128 * BLK_SIZE as u128
    }
}
