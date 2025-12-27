use ftl_driver::device::BusAddr;
use ftl_driver::device::DmaDesc;

use crate::DeviceType;
use crate::virtqueue::VirtQueue;

pub mod mmio;

pub use mmio::VirtioMmio;

#[derive(Debug, Copy, Clone)]
#[repr(transparent)]
pub struct IsrStatus(pub u8);

const QUEUE_INTR: u8 = 1 << 0;
const DEVICE_CONFIG_INTR: u8 = 1 << 1;

impl IsrStatus {
    pub fn is_queue_interrupt(&self) -> bool {
        (self.0 & QUEUE_INTR) != 0
    }

    pub fn is_device_config_interrupt(&self) -> bool {
        (self.0 & DEVICE_CONFIG_INTR) != 0
    }
}

pub trait Transport {
    fn device_type(&self) -> DeviceType;
    fn negotiate_features(&self, features: u64);
    fn initialize(&self);
    fn virtqueue(
        &self,
        dma_desc: DmaDesc,
        index: u16,
    ) -> Result<VirtQueue, crate::virtqueue::Error>;
    fn read_isr_status(&self) -> IsrStatus;

    // Low-level interfaces.
    fn select_queue(&self, index: u16);
    fn queue_max_size(&self) -> u16;
    fn set_queue_size(&self, queue_size: u16);
    fn notify_queue(&self, index: u16);
    fn enable_queue(&self);
    fn set_queue_desc_paddr(&self, baddr: BusAddr);
    fn set_queue_driver_paddr(&self, baddr: BusAddr);
    fn set_queue_device_paddr(&self, baddr: BusAddr);
}
