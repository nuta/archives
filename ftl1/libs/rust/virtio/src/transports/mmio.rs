use ftl_driver::device::BusAddr;
use ftl_driver::device::DmaDesc;
use ftl_driver::iospace::DeviceReg;
use ftl_driver::iospace::IoSpace;
use ftl_driver::iospace::LittleEndian;
use ftl_driver::iospace::ReadOnly;
use ftl_driver::iospace::ReadWrite;
use ftl_driver::iospace::WriteOnly;

use super::Transport;
use crate::DeviceType;
use crate::transports::IsrStatus;
use crate::virtqueue::VirtQueue;

// "All register values are organized as Little Endian."
// (4.2.2 MMIO Device Register Layout).
const MAGIC_VALUE_REG: DeviceReg<LittleEndian, ReadOnly, u32> = DeviceReg::new(0x00);
const DEVICE_VERSION_REG: DeviceReg<LittleEndian, ReadOnly, u32> = DeviceReg::new(0x04);
const DEVICE_ID_REG: DeviceReg<LittleEndian, ReadOnly, u32> = DeviceReg::new(0x08);
const DEVICE_FEATURES_REG: DeviceReg<LittleEndian, ReadOnly, u32> = DeviceReg::new(0x10);
const DEVICE_FEATURES_SEL_REG: DeviceReg<LittleEndian, WriteOnly, u32> = DeviceReg::new(0x14);
const DRIVER_FEATURES_REG: DeviceReg<LittleEndian, WriteOnly, u32> = DeviceReg::new(0x20);
const DRIVER_FEATURES_SEL_REG: DeviceReg<LittleEndian, WriteOnly, u32> = DeviceReg::new(0x24);
const QUEUE_SEL_REG: DeviceReg<LittleEndian, WriteOnly, u32> = DeviceReg::new(0x30);
const QUEUE_NUM_MAX_REG: DeviceReg<LittleEndian, ReadOnly, u32> = DeviceReg::new(0x34);
const QUEUE_NUM_REG: DeviceReg<LittleEndian, WriteOnly, u32> = DeviceReg::new(0x38);
const QUEUE_READY_REG: DeviceReg<LittleEndian, ReadWrite, u32> = DeviceReg::new(0x44);
const QUEUE_NOTIFY_REG: DeviceReg<LittleEndian, WriteOnly, u32> = DeviceReg::new(0x50);
const INTERRUPT_STATUS_REG: DeviceReg<LittleEndian, ReadOnly, u32> = DeviceReg::new(0x60);
const INTERRUPT_ACK_REG: DeviceReg<LittleEndian, WriteOnly, u32> = DeviceReg::new(0x64);
const DEVICE_STATUS_REG: DeviceReg<LittleEndian, ReadWrite, u32> = DeviceReg::new(0x70);
const QUEUE_DESC_LOW_REG: DeviceReg<LittleEndian, WriteOnly, u32> = DeviceReg::new(0x80);
const QUEUE_DESC_HIGH_REG: DeviceReg<LittleEndian, WriteOnly, u32> = DeviceReg::new(0x84);
const QUEUE_DRIVER_LOW_REG: DeviceReg<LittleEndian, WriteOnly, u32> = DeviceReg::new(0x90);
const QUEUE_DRIVER_HIGH_REG: DeviceReg<LittleEndian, WriteOnly, u32> = DeviceReg::new(0x94);
const QUEUE_DEVICE_LOW_REG: DeviceReg<LittleEndian, WriteOnly, u32> = DeviceReg::new(0xa0);
const QUEUE_DEVICE_HIGH_REG: DeviceReg<LittleEndian, WriteOnly, u32> = DeviceReg::new(0xa4);
const CONFIG_REG_BASE: usize = 0x100;

#[derive(Debug)]
pub enum ProbeError {
    InvalidMagic(u32),
    UnsupportedVersion(u32),
}

pub struct VirtioMmio {
    iospace: IoSpace,
}

const STATUS_ACKNOWLEDGE: u32 = 1;
const STATUS_DRIVER: u32 = 2;
const STATUS_DRIVER_OK: u32 = 4;
const STATUS_FEATURES_OK: u32 = 8;

impl VirtioMmio {
    pub fn probe(iospace: IoSpace) -> Result<Self, ProbeError> {
        // Check if the device is present by checking t he magic value
        // ("virt" in little-endian).
        let magic = iospace.read32(MAGIC_VALUE_REG);
        if magic != 0x74726976 {
            return Err(ProbeError::InvalidMagic(magic));
        }

        let version = iospace.read32(DEVICE_VERSION_REG);
        if version != 2 {
            return Err(ProbeError::UnsupportedVersion(version));
        }

        // Reset the device.
        iospace.write32(DEVICE_STATUS_REG, 0);

        Ok(VirtioMmio { iospace })
    }

    fn set_device_status(&self, bits: u32) {
        let value = self.iospace.read32(DEVICE_STATUS_REG);
        self.iospace.write32(DEVICE_STATUS_REG, value | bits);
    }

    pub fn read_config(&self, offset: usize, data: &mut [u8]) {
        for (i, byte) in data.iter_mut().enumerate() {
            let cfg_offset = CONFIG_REG_BASE + offset + i;
            let aligned = cfg_offset & !0x3;
            let shift = (cfg_offset & 0x3) * 8;

            let value = self
                .iospace
                .read32(DeviceReg::<LittleEndian, ReadOnly, u32>::new(aligned));
            *byte = ((value >> shift) & 0xff) as u8;
        }
    }
}

impl Transport for VirtioMmio {
    fn device_type(&self) -> DeviceType {
        let device_type = self.iospace.read32(DEVICE_ID_REG);
        match device_type {
            1 => DeviceType::Net,
            2 => DeviceType::Blk,
            _ => DeviceType::Unknown,
        }
    }

    fn negotiate_features(&self, features: u64) {
        // "3.1.1 Driver Requirements: Device Initialization"
        self.set_device_status(STATUS_ACKNOWLEDGE);
        self.set_device_status(STATUS_DRIVER);

        self.iospace.write32(DEVICE_FEATURES_SEL_REG, 0);
        let low = self.iospace.read32(DEVICE_FEATURES_REG);
        self.iospace.write32(DEVICE_FEATURES_SEL_REG, 1);
        let high = self.iospace.read32(DEVICE_FEATURES_REG);
        let device_features = (high as u64) << 32 | (low as u64);

        if (device_features & features) != features {
            // TODO: error
            panic!(
                "virtio: feature negotiation failure: driver={:x}, device={:x}, unspported={:x}",
                features,
                device_features,
                features & !device_features
            );
        }

        self.iospace.write32(DRIVER_FEATURES_SEL_REG, 0);
        self.iospace.write32(DRIVER_FEATURES_REG, features as u32);
        self.iospace.write32(DRIVER_FEATURES_SEL_REG, 1);
        self.iospace
            .write32(DRIVER_FEATURES_REG, (features >> 32) as u32);

        self.set_device_status(STATUS_FEATURES_OK);

        if (self.iospace.read32(DEVICE_STATUS_REG) & STATUS_FEATURES_OK) == 0 {
            panic!(
                "virtio: feature negotiation failure: device={:x}",
                self.iospace.read32(DEVICE_STATUS_REG)
            );
        }
    }

    fn initialize(&self) {
        self.set_device_status(STATUS_DRIVER_OK);
    }

    fn virtqueue(
        &self,
        dma_desc: DmaDesc,
        index: u16,
    ) -> Result<VirtQueue, crate::virtqueue::Error> {
        VirtQueue::new(index, dma_desc, self)
    }

    fn read_isr_status(&self) -> IsrStatus {
        let raw = self.iospace.read32(INTERRUPT_STATUS_REG);
        if raw != 0 {
            self.iospace.write32(INTERRUPT_ACK_REG, raw);
        }
        IsrStatus(raw as u8)
    }

    fn select_queue(&self, index: u16) {
        self.iospace.write32(QUEUE_SEL_REG, index as u32);
    }

    fn queue_max_size(&self) -> u16 {
        self.iospace.read32(QUEUE_NUM_MAX_REG) as u16
    }

    fn set_queue_size(&self, queue_size: u16) {
        self.iospace.write32(QUEUE_NUM_REG, queue_size as u32);
    }

    fn notify_queue(&self, index: u16) {
        self.iospace.write32(QUEUE_NOTIFY_REG, index as u32);
    }

    fn enable_queue(&self) {
        self.iospace.write32(QUEUE_READY_REG, 1);
    }

    fn set_queue_desc_paddr(&self, baddr: BusAddr) {
        self.iospace
            .write32(QUEUE_DESC_LOW_REG, baddr.lower_32bits());
        self.iospace
            .write32(QUEUE_DESC_HIGH_REG, baddr.upper_32bits());
    }

    fn set_queue_driver_paddr(&self, baddr: BusAddr) {
        self.iospace
            .write32(QUEUE_DRIVER_LOW_REG, baddr.lower_32bits());
        self.iospace
            .write32(QUEUE_DRIVER_HIGH_REG, baddr.upper_32bits());
    }

    fn set_queue_device_paddr(&self, baddr: BusAddr) {
        self.iospace
            .write32(QUEUE_DEVICE_LOW_REG, baddr.lower_32bits());
        self.iospace
            .write32(QUEUE_DEVICE_HIGH_REG, baddr.upper_32bits());
    }
}
