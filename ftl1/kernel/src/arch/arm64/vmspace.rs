use core::arch::asm;
use core::mem::size_of;
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
const MAX_LEVEL: usize = 2;

const PTE_VALID: u64 = 1 << 0;
const PTE_TABLE: u64 = 1 << 1;
const PTE_TYPE_MASK: u64 = 0b11;
const PTE_TYPE_BLOCK: u64 = 0b01;
const PTE_TYPE_TABLE_OR_PAGE: u64 = 0b11;
const PTE_AF: u64 = 1 << 10;
const PTE_SH_INNER: u64 = 3 << 8;
const PTE_ATTR_IDX: u64 = 0 << 2;

// 39-bits virtual address space because block descriptors are not supported
// at the level 0 table (i.e. 512GiB pages).
pub const KERNEL_BASE_ADDR: VAddr = VAddr::new(0xffff_ffe0_0000_0000);
pub const UADDR_RANGE: Range<UAddr> = UAddr::new(0x10000)..UAddr::new(KERNEL_BASE_ADDR.as_usize());
pub const MIN_PAGE_SIZE: usize = 0x1000;

const GIGA_PAGE_SIZE: usize = 1 << 30;
const DIRECT_MAPPING_SIZE: usize = GIGA_PAGE_SIZE * 16;

static_assert!(get_index(KERNEL_BASE_ADDR, 2) == 384);

#[derive(Debug, Clone, Copy)]
#[repr(transparent)]
pub struct Pte(u64);

impl Pte {
    pub const fn new(paddr: PAddr, attrs: PageAttrs) -> Self {
        assert!(is_aligned(paddr.as_usize(), 4096));

        let oa = (paddr.as_usize() as u64) & 0x0000_ffff_ffff_f000;
        Self(oa | attrs.bits() | PTE_TABLE)
    }

    pub const fn new_block(paddr: PAddr, attrs: PageAttrs) -> Self {
        assert!(is_aligned(paddr.as_usize(), GIGA_PAGE_SIZE));

        let oa = (paddr.as_usize() as u64) & 0x0000_ffff_c000_0000;
        Self(oa | attrs.bits() | PTE_VALID | PTE_AF | PTE_SH_INNER | PTE_ATTR_IDX)
    }

    pub const fn new_table(paddr: PAddr) -> Self {
        let oa = (paddr.as_usize() as u64) & 0x0000_ffff_ffff_f000;
        Self(oa | PTE_VALID | PTE_TABLE)
    }

    pub const fn set(&mut self, paddr: PAddr, attrs: PageAttrs) {
        *self = Pte::new(paddr, attrs);
    }

    pub fn is_valid(&self) -> bool {
        self.0 & PTE_VALID != 0
    }

    fn descriptor_type(&self) -> u64 {
        self.0 & PTE_TYPE_MASK
    }

    pub fn is_block(&self) -> bool {
        self.is_valid() && self.descriptor_type() == PTE_TYPE_BLOCK
    }

    pub fn is_table_descriptor_at_level(&self, level: usize) -> bool {
        level > 0 && self.is_valid() && self.descriptor_type() == PTE_TYPE_TABLE_OR_PAGE
    }

    pub fn paddr(&self) -> PAddr {
        let raw = (self.0 & 0x0000_ffff_ffff_f000) as usize;
        PAddr::new(raw)
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
            let new_table = PAGE_ALLOCATOR.allocate_pages_zeroed(size_of::<Table>())?;
            *entry = Pte::new_table(new_table);
        }

        if entry.is_block() {
            return Ok(TableOrLeaf::Leaf);
        }

        debug_assert!(entry.is_table_descriptor_at_level(level));
        let child = paddr2table(entry.paddr());
        Ok(TableOrLeaf::Table(child))
    }
}

pub(super) static TMP_PAGE_TABLE: Table = {
    let mut table = Table([Pte(0); ENTRIES_PER_TABLE]);
    let attrs = PageAttrs::valid().read().write().execute();

    // Map the first 4GiB physical memory. It should be plenty enough.
    let mut i = 0;
    while i < 4 {
        let offset = i * GIGA_PAGE_SIZE;
        let paddr = PAddr::new(offset);
        let pte = Pte::new_block(paddr, attrs);
        *table.get_mut(get_index(VAddr::new(offset), 2)) = pte;
        *table.get_mut(get_index(KERNEL_BASE_ADDR.add(offset), 2)) = pte;
        i += 1;
    }

    table
};

#[track_caller]
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

fn free_table(table_paddr: PAddr, level: usize) {
    let table = paddr2table(table_paddr);
    if level > 0 {
        for i in 0..ENTRIES_PER_TABLE {
            let pte = &mut table.get_mut(i);
            if pte.is_table_descriptor_at_level(level) {
                free_table(pte.paddr(), level - 1);
            }
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

        let table = paddr2table(paddr);
        let attrs = PageAttrs::valid().read().write().execute();

        for offset in (0..DIRECT_MAPPING_SIZE).step_by(GIGA_PAGE_SIZE) {
            let vaddr = KERNEL_BASE_ADDR.add(offset);
            let index = get_index(vaddr, 2);
            let pte = Pte::new_block(PAddr::new(offset), attrs);
            *table.get_mut(index) = pte;
        }

        Ok(Self { paddr })
    }

    pub fn switch(&self) {
        let old_ttbr: u64;
        unsafe {
            asm!("mrs {}, ttbr0_el1", out(reg) old_ttbr);
        }

        let new_ttbr = self.paddr.as_usize() as u64;

        if old_ttbr == new_ttbr {
            return;
        }

        unsafe {
            asm!(
                "dsb ishst",
                "msr ttbr0_el1, {ttbr}",
                "msr ttbr1_el1, {ttbr}",
                "isb",
                "tlbi vmalle1is",
                "dsb ish",
                "isb",
                ttbr = in(reg) new_ttbr,
            );
        }
    }

    fn walk(&self, vaddr: VAddr) -> Result<&mut Pte, ErrorCode> {
        let mut table = paddr2table(self.paddr);
        for level in (1..=MAX_LEVEL).rev() {
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

        debug_assert!(!pte.is_valid(), "page already mapped");

        pte.set(paddr, attrs);
        unsafe {
            asm!("dsb ishst");
            asm!("tlbi vaae1is, {}", in(reg) vaddr.as_usize() >> 12);
            asm!("dsb ish");
            asm!("isb");
        }

        Ok(())
    }
}

impl Drop for VmSpace {
    fn drop(&mut self) {
        free_table(self.paddr, MAX_LEVEL);
    }
}
