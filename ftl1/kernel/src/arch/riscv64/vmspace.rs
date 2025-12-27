use core::arch::asm;
use core::ops::Range;

use ftl_types::arch::PageAttrs;
use ftl_types::error::ErrorCode;
use ftl_utils::alignment::is_aligned;
use ftl_utils::static_assert;

use crate::address::PAddr;
use crate::address::UAddr;
use crate::address::VAddr;
use crate::memory::PAGE_ALLOCATOR;

const ENTRIES_PER_TABLE: usize = 512;
pub const PPN_SHIFT: usize = 12;

pub const SATP_MODE_SV48: u64 = 9 << 60;

const PTE_V: u64 = 1 << 0;
const PTE_R: u64 = 1 << 1;
const PTE_W: u64 = 1 << 2;
const PTE_X: u64 = 1 << 3;
const PTE_PPN_SHIFT: usize = 10;

pub const KERNEL_BASE_ADDR: VAddr = VAddr::new(0xffff_ff80_0000_0000);
pub const UADDR_RANGE: Range<UAddr> = UAddr::new(0x10000)..UAddr::new(KERNEL_BASE_ADDR.as_usize());
pub const MIN_PAGE_SIZE: usize = 0x1000;

const TERA_PAGE_SIZE: usize = 1 << 39;
const DIRECT_MAPPING_SIZE: usize = TERA_PAGE_SIZE; // 512 GiB

static_assert!(get_index(KERNEL_BASE_ADDR, 3) == 511);

#[derive(Debug, Clone, Copy)]
#[repr(transparent)]
pub struct Pte(u64);

impl Pte {
    pub const fn new(paddr: PAddr, attrs: PageAttrs) -> Self {
        assert!(is_aligned(paddr.as_usize(), 4096));

        let ppn = paddr.as_usize() as u64 >> PPN_SHIFT;
        Self(ppn << PTE_PPN_SHIFT | attrs.bits())
    }

    pub const fn set(&mut self, paddr: PAddr, attrs: PageAttrs) {
        self.0 = Pte::new(paddr, attrs).0;
    }

    pub fn is_valid(&self) -> bool {
        self.0 & PTE_V != 0
    }

    pub fn is_leaf(&self) -> bool {
        self.0 & (PTE_R | PTE_W | PTE_X) != 0
    }

    pub fn ppn(&self) -> u64 {
        self.0 >> PTE_PPN_SHIFT
    }

    pub fn paddr(&self) -> PAddr {
        let raw = self.ppn() << PPN_SHIFT;
        PAddr::new(raw as usize)
    }
}

enum TableOrLeaf<'a> {
    Table(&'a mut Table),
    Leaf,
}

#[repr(align(4096))]
pub struct Table(pub(super) [Pte; ENTRIES_PER_TABLE]);

static_assert!(size_of::<Table>() == 4096);

impl Table {
    pub const fn get_mut(&mut self, index: usize) -> &mut Pte {
        &mut self.0[index]
    }

    fn get_child_table(
        &mut self,
        vaddr: VAddr,
        level: usize,
    ) -> Result<TableOrLeaf<'_>, ErrorCode> {
        let index = get_index(vaddr, level);
        let entry = &mut self.0[index];

        if !entry.is_valid() {
            // Allocate a new table.
            let new_table = PAGE_ALLOCATOR.allocate_pages_zeroed(size_of::<Table>())?;
            let attrs = PageAttrs::valid();
            *entry = Pte::new(new_table, attrs);
        }

        if entry.is_leaf() {
            return Ok(TableOrLeaf::Leaf);
        }

        let child = paddr2table(entry.paddr());
        Ok(TableOrLeaf::Table(child))
    }
}

pub(super) static TMP_PAGE_TABLE: Table = {
    let mut table = Table([Pte(0); ENTRIES_PER_TABLE]);
    let attrs = PageAttrs::valid().read().write().execute();

    table
        .get_mut(get_index(VAddr::new(0), 3))
        .set(PAddr::new(0), attrs);

    table
        .get_mut(get_index(KERNEL_BASE_ADDR, 3))
        .set(PAddr::new(0), attrs);

    table
};

/// Returns a kernel virtual address mapped to the given physical address.
///
/// This will never fail because the entire physical memory is mapped to the
/// kernel's address space (so-called direct mapping).
pub fn paddr2vaddr(paddr: PAddr) -> VAddr {
    VAddr::new(paddr.as_usize() + KERNEL_BASE_ADDR.as_usize())
}

#[track_caller]
pub fn vaddr2paddr(vaddr: VAddr) -> PAddr {
    assert!(vaddr.as_usize() >= KERNEL_BASE_ADDR.as_usize());
    PAddr::new(vaddr.as_usize() - KERNEL_BASE_ADDR.as_usize())
}

const fn get_index(vaddr: VAddr, level: usize) -> usize {
    debug_assert!(level <= 3);
    (vaddr.as_usize() >> (12 + 9 * level)) & 0x1ff
}

fn paddr2table(paddr: PAddr) -> &'static mut Table {
    let vaddr = paddr2vaddr(paddr);
    unsafe { &mut *vaddr.as_mut_ptr() }
}

/// Frees a page table recursively.
fn free_table(table_paddr: PAddr) {
    let table = paddr2table(table_paddr);
    for i in 0..ENTRIES_PER_TABLE {
        let pte = &mut table.get_mut(i);
        if pte.is_valid() && !pte.is_leaf() {
            free_table(pte.paddr());
        }
    }

    PAGE_ALLOCATOR
        .free_pages(table_paddr, size_of::<Table>())
        .unwrap();
}

pub struct VmSpace {
    paddr: PAddr,
}

impl VmSpace {
    pub fn new() -> Result<Self, ErrorCode> {
        let paddr = PAGE_ALLOCATOR.allocate_pages_zeroed(size_of::<Table>())?;
        let attrs = PageAttrs::valid().read().write().execute();

        // Map the kernel space.
        let table = paddr2table(paddr);
        for offset in (0..DIRECT_MAPPING_SIZE).step_by(TERA_PAGE_SIZE) {
            let vaddr = KERNEL_BASE_ADDR.add(offset);
            let paddr = PAddr::new(offset);
            table.get_mut(get_index(vaddr, 3)).set(paddr, attrs);
        }

        Ok(Self { paddr })
    }

    pub fn switch(&self) {
        let old_satp: u64;
        unsafe {
            asm!("csrr {}, satp", out(reg) old_satp);
        }

        let new_satp = (self.paddr.as_usize() as u64 >> PPN_SHIFT) | SATP_MODE_SV48;

        // Do nothing if the current CPU is already in the same page table so
        // that we don't flush the TLB needlessly.
        if old_satp == new_satp {
            return;
        }

        unsafe {
            // Do sfeence.vma before and even before switching the page
            // table to ensure all changes prior to this switch are visible.
            //
            // (The RISC-V Instruction Set Manual Volume II, Version 1.10, p. 58)
            asm!(
                "sfence.vma",
                "csrw satp, {}",
                "sfence.vma",
                in(reg) new_satp,
            );
        }
    }

    /// Walks into the last level of the page table.
    fn walk(&self, vaddr: VAddr) -> Result<&mut Pte, ErrorCode> {
        let mut table = paddr2table(self.paddr);
        for level in (1..=3).rev() {
            table = match table.get_child_table(vaddr, level) {
                Ok(TableOrLeaf::Table(table)) => table,
                Ok(TableOrLeaf::Leaf) => return Err(ErrorCode::AlreadyMapped),
                Err(e) => return Err(e),
            };
        }

        let entry = table.get_mut(get_index(vaddr, 0));
        Ok(entry)
    }

    pub fn map(&self, vaddr: VAddr, paddr: PAddr, attrs: PageAttrs) -> Result<(), ErrorCode> {
        let pte = self.walk(vaddr)?;
        pte.set(paddr, attrs);

        unsafe {
            // Flush the TLB for the modified page.
            asm!("sfence.vma {}, zero", in(reg) vaddr.as_usize());
        }

        Ok(())
    }
}

impl Drop for VmSpace {
    fn drop(&mut self) {
        free_table(self.paddr);
    }
}
