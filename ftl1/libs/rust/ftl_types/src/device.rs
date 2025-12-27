use alloc::vec::Vec;

/// Defines the constraints for a DMA buffer.
#[derive(Debug, Clone, Copy, serde::Serialize, serde::Deserialize)]
pub enum DmaDesc {
    /// No IOMMUs. That is `BusAddr == PAddr`.
    Direct,
}

/// Defines memory-mapped/port-mapped device registers mapping.
#[derive(Debug, Clone, Copy, serde::Serialize, serde::Deserialize)]
pub enum IoSpaceDesc {
    Mmio { baddr: BusAddr, size: usize },
}

#[derive(
    Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord, Hash, serde::Serialize, serde::Deserialize,
)]
#[repr(transparent)]
pub struct BusAddr(usize);

impl BusAddr {
    pub const fn new(addr: usize) -> Self {
        Self(addr)
    }

    pub fn lower_32bits(self) -> u32 {
        (self.0 & 0xffff_ffff) as u32
    }

    pub fn upper_32bits(self) -> u32 {
        (self.0 >> 32) as u32
    }

    pub fn as_usize(self) -> usize {
        self.0
    }

    pub fn offset(self, offset: usize) -> Self {
        Self(self.0 + offset)
    }
}

#[derive(Debug, Clone, Copy, serde::Serialize, serde::Deserialize)]
pub struct IrqDesc {
    n: usize,
}

impl IrqDesc {
    pub fn new(n: usize) -> Self {
        Self { n }
    }

    pub fn as_raw(&self) -> usize {
        self.n
    }
}

#[derive(Debug, Clone, serde::Serialize, serde::Deserialize)]
pub struct DeviceMatch {
    pub iospace: IoSpaceDesc,
    pub dma: DmaDesc,
    pub irq: IrqDesc,
}

#[derive(Debug, Clone, serde::Serialize, serde::Deserialize)]
pub struct DeviceDesc {
    pub matches: Vec<DeviceMatch>,
}
