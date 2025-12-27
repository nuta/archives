use alloc::vec;
use alloc::vec::Vec;
use core::ops::Range;

use arrayvec::ArrayVec;
use ftl_dtb_parser::Dtb;
use ftl_utils::alignment::align_down;
use ftl_utils::alignment::align_up;
use ftl_utils::byte_size::ByteSize;

use crate::address::PAddr;
use crate::address::VAddr;
use crate::arch::MIN_PAGE_SIZE;
use crate::arch::vaddr2paddr;
use crate::memory::PAGE_ALLOCATOR;

pub fn get_free_ram_areas(
    memory: &[Range<PAddr>],
    reserved_memory: &[Range<PAddr>],
) -> ArrayVec<FreeRam, 32> {
    let mut free_areas = ArrayVec::new();

    for mem_range in memory {
        let mut current_ranges = vec![mem_range.clone()];

        for reserved in reserved_memory {
            let mut new_ranges = Vec::new();
            for range in current_ranges {
                if reserved.end <= range.start || reserved.start >= range.end {
                    new_ranges.push(range);
                } else {
                    if range.start < reserved.start {
                        new_ranges.push(range.start..reserved.start);
                    }
                    if reserved.end < range.end {
                        new_ranges.push(reserved.end..range.end);
                    }
                }
            }
            current_ranges = new_ranges;
        }

        for range in current_ranges {
            free_areas.push(FreeRam {
                paddr: range.start,
                size: range.end.as_usize() - range.start.as_usize(),
            });
        }
    }

    free_areas
}

unsafe extern "C" {
    static __kernel_memory: u8;
    static __kernel_memory_end: u8;
}

#[derive(Debug, Clone, Copy)]
pub struct FreeRam {
    pub paddr: PAddr,
    pub size: usize,
}

pub fn init(dtb_vaddr: VAddr) -> Dtb<'static> {
    let dtb = unsafe { Dtb::parse_from_ptr(dtb_vaddr.as_mut_ptr()) }
        .expect("failed to parse device tree");
    let mut reserved_memory: ArrayVec<Range<PAddr>, 16> = ArrayVec::new();
    let mut memory: ArrayVec<Range<PAddr>, 16> = ArrayVec::new();

    // Parse memory nodes.
    for node in dtb.nodes() {
        let node = node.expect("failed to get node");
        match node.name() {
            name if name.starts_with(b"memory@") => {
                let Some(range) = node.reg().expect("failed to get reg") else {
                    continue;
                };

                let start = PAddr::new(range.addr as usize);
                let end = PAddr::new((range.addr + range.size) as usize);
                memory.push(start..end);
            }
            name if node.is_reserved_memory_node() => {
                let Some(range) = node.reg().expect("failed to get reg") else {
                    continue;
                };

                let start = PAddr::new(range.addr as usize);
                let end = PAddr::new((range.addr + range.size) as usize);
                reserved_memory.push(start..end);
            }
            _ => {}
        }
    }

    // Exclude the kernel memory area from the free RAM areas.
    let kernel_start = vaddr2paddr(VAddr::new(&raw const __kernel_memory as usize));
    let kernel_end = vaddr2paddr(VAddr::new(&raw const __kernel_memory_end as usize));
    reserved_memory.push(kernel_start..kernel_end);

    // Exclude the device tree from the free RAM areas.
    let dtb_start = vaddr2paddr(dtb_vaddr);
    let dtb_end = vaddr2paddr(dtb_vaddr.add(dtb.len()));
    reserved_memory.push(dtb_start..dtb_end);

    for area in &mut reserved_memory {
        let start = align_down(area.start.as_usize(), MIN_PAGE_SIZE);
        let end = align_up(area.end.as_usize(), MIN_PAGE_SIZE);
        *area = PAddr::new(start)..PAddr::new(end);

        debug!(
            "reserved area: {} - {} ({})",
            area.start,
            area.end,
            ByteSize(area.end.as_usize() - area.start.as_usize())
        );
    }

    for area in get_free_ram_areas(&memory, &reserved_memory) {
        info!("free RAM area: {} ({})", area.paddr, ByteSize(area.size));
        PAGE_ALLOCATOR.add_region(area.paddr, area.size);
    }

    dtb
}
