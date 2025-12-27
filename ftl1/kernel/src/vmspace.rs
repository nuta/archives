use core::ops::Range;

use ftl_types::error::ErrorCode;
use ftl_types::handle::HandleId;
use ftl_types::handle::HandleRight;
use ftl_utils::alignment::align_down;
use ftl_utils::range_map::RangeMap;

use crate::address::UAddr;
use crate::address::VAddr;
use crate::arch;
use crate::arch::MIN_PAGE_SIZE;
use crate::arch::PageAttrs;
use crate::arch::UADDR_RANGE;
use crate::handle::Handleable;
use crate::shared_ref::SharedRef;
use crate::spinlock::SpinLock;
use crate::syscall::SyscallResult;
use crate::thread::Thread;
use crate::vmarea::VmArea;

struct Mapping {
    vmarea: SharedRef<VmArea>,
    attrs: PageAttrs,
}

pub struct VmSpace {
    arch: arch::VmSpace,
    vmareas: SpinLock<RangeMap<UAddr, Mapping>>,
}

impl VmSpace {
    pub fn new() -> Result<Self, ErrorCode> {
        let arch = arch::VmSpace::new()?;
        Ok(Self {
            arch,
            vmareas: SpinLock::new(RangeMap::new()),
        })
    }

    pub fn insert_at(
        &self,
        range: Range<UAddr>,
        vmarea: SharedRef<VmArea>,
        attrs: PageAttrs,
    ) -> Result<(), ErrorCode> {
        let mut vmareas = self.vmareas.lock();
        vmareas
            .insert(range, Mapping { vmarea, attrs })
            .map_err(|_| ErrorCode::OutOfSpace)?;

        Ok(())
    }

    pub fn insert_anywhere(
        &self,
        len: usize,
        vmarea: SharedRef<VmArea>,
        attrs: PageAttrs,
    ) -> Result<Range<UAddr>, ErrorCode> {
        let mut vmareas = self.vmareas.lock();

        let range = vmareas
            .insert_anywhere(len, UADDR_RANGE, Mapping { vmarea, attrs })
            .map_err(|_| ErrorCode::OutOfSpace)?;

        Ok(range)
    }

    pub fn handle_page_fault(&self, uaddr: UAddr) -> Result<(), ErrorCode> {
        let vmareas = self.vmareas.lock();
        let (range, mapping) = vmareas.get(uaddr).ok_or(ErrorCode::SegmentationFault)?;

        let offset_unaligned = uaddr.as_usize() - range.start.as_usize();
        let offset = align_down(offset_unaligned, MIN_PAGE_SIZE);
        let paddr = mapping.vmarea.fill(offset)?; // TODO: What if this would block (pager-backed)?
        let vaddr = VAddr::new(range.start.as_usize() + offset);
        self.arch.map(vaddr, paddr, mapping.attrs)?;
        Ok(())
    }

    pub fn switch(&self) {
        self.arch.switch();
    }
}

impl Handleable for VmSpace {}

pub fn sys_vmspace_map(
    current: &SharedRef<Thread>,
    a0: usize,
    a1: usize,
    a2: usize,
    a3: usize,
    a4: usize,
    a5: usize,
) -> Result<SyscallResult, ErrorCode> {
    let vmspace_id = HandleId::from_usize(a0)?;
    let vmarea_id = HandleId::from_usize(a1)?;
    let uaddr = UAddr::new(a2);
    let offset = a3;
    let len = a4;
    let attrs = PageAttrs::from_usize(a5);

    let handle_table = current.process().handles().lock();
    let vmspace = handle_table
        .get::<VmSpace>(vmspace_id)?
        .authorize(HandleRight::WRITE)?;
    let vmarea = handle_table
        .get::<VmArea>(vmarea_id)?
        .authorize(HandleRight::WRITE)?;

    if offset != 0 {
        warn!("offset is not supported yet");
        return Err(ErrorCode::NotSupported);
    }

    let start = if uaddr.as_usize() == 0 {
        let range = vmspace.insert_anywhere(len, vmarea, attrs)?;
        range.start
    } else {
        vmspace.insert_at(uaddr..uaddr.add(len), vmarea, attrs)?;
        uaddr
    };

    Ok(SyscallResult::Return(start.as_usize()))
}

pub static KERNEL_VMSPACE: spin::Lazy<SharedRef<VmSpace>> = spin::Lazy::new(|| {
    let vmspace = VmSpace::new().unwrap();
    SharedRef::new(vmspace).unwrap()
});

pub fn init() {
    // We're currently using a temporary page table for bootstrapping. Switch
    // to the page table we've prepared for the kernel.
    KERNEL_VMSPACE.switch();
}
