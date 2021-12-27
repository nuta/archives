use arrayvec::ArrayVec;

use crate::{paddr::PAddr, vaddr::VAddr};

const NUM_FREE_RAM_AREAS_MAX: usize = 8;

/// A continuous physical memory space available for OS.
#[derive(Debug)]
pub struct FreeRamArea {
    /// The base vaddr.
    pub vaddr: VAddr,
    /// The base paddr.
    pub paddr: PAddr,
    /// The length in bytes.
    pub len: usize,
}

/// The boot information for kernel.
#[derive(Debug)]
pub struct BootInfo {
    pub free_ram_areas: ArrayVec<FreeRamArea, NUM_FREE_RAM_AREAS_MAX>,
}
