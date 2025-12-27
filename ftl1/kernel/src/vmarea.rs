use core::mem::size_of;

use ftl_types::error::ErrorCode;
use ftl_types::handle::HandleRight;
use ftl_utils::alignment::is_aligned;
use ftl_utils::range_map::RangeMap;

use crate::address::PAddr;
use crate::arch::MIN_PAGE_SIZE;
use crate::handle::Handle;
use crate::handle::Handleable;
use crate::isolation::IsolationPtr;
use crate::isolation::IsolationSliceMut;
use crate::memory::PAGE_ALLOCATOR;
use crate::shared_ref::SharedRef;
use crate::spinlock::SpinLock;
use crate::syscall::SyscallResult;
use crate::thread::Thread;

/// Memory pages that are contiguous in the physical address space.
#[derive(Debug)]
struct Page {
    /// The base address of the contiguous pages.
    paddr: PAddr,
}

impl Drop for Page {
    fn drop(&mut self) {
        if let Err(err) = PAGE_ALLOCATOR.free_pages(self.paddr, MIN_PAGE_SIZE) {
            debug_warn!("failed to free page: {:?}", err);
        }
    }
}

enum Strategy {
    Any,
    Pinned(PAddr),
}

struct Mutable {
    pages: RangeMap<usize /* byte offset */, Page>,
}

pub struct VmArea {
    size: usize,
    mutable: SpinLock<Mutable>,
    strategy: Strategy,
}

impl VmArea {
    pub fn new_any(size: usize) -> Self {
        Self {
            size,
            mutable: SpinLock::new(Mutable {
                pages: RangeMap::new(),
            }),
            strategy: Strategy::Any,
        }
    }

    pub fn new_pinned(paddr: PAddr, size: usize) -> Self {
        Self {
            size,
            mutable: SpinLock::new(Mutable {
                pages: RangeMap::new(),
            }),
            strategy: Strategy::Pinned(paddr),
        }
    }

    pub fn fill(&self, offset: usize) -> Result<PAddr, ErrorCode> {
        if offset + MIN_PAGE_SIZE > self.size {
            return Err(ErrorCode::OutOfBounds);
        }

        let paddr = match self.strategy {
            Strategy::Any => {
                let start = PAGE_ALLOCATOR.allocate_pages_zeroed(MIN_PAGE_SIZE)?;
                let range = offset..(offset + MIN_PAGE_SIZE);
                let mut mutable = self.mutable.lock();
                mutable
                    .pages
                    .insert(range, Page { paddr: start })
                    .expect("new page overlaps with an existing page in vmarea");

                start
            }
            Strategy::Pinned(paddr) => paddr.checked_add(offset).ok_or(ErrorCode::OutOfBounds)?,
        };

        Ok(paddr)
    }
}

impl Handleable for VmArea {}

pub fn sys_vmarea_alloc(
    current: &SharedRef<Thread>,
    a0: usize,
) -> Result<SyscallResult, ErrorCode> {
    let size = a0;

    let vmarea = VmArea::new_any(size);
    let handle = Handle::new(SharedRef::new(vmarea)?, HandleRight::WRITE);
    let handle_id = current.process().handles().lock().insert(handle)?;

    Ok(SyscallResult::Return(handle_id.as_usize()))
}

pub fn sys_vmarea_pin(
    current: &SharedRef<Thread>,
    a0: usize,
    a1: usize,
) -> Result<SyscallResult, ErrorCode> {
    let paddr = PAddr::new(a0);
    let size = a1;

    // Validate size is page-aligned
    if !is_aligned(size, MIN_PAGE_SIZE) {
        return Err(ErrorCode::InvalidArgument);
    }

    let vmarea = VmArea::new_pinned(paddr, size);
    let handle = Handle::new(SharedRef::new(vmarea)?, HandleRight::WRITE);
    let handle_id = current.process().handles().lock().insert(handle)?;

    Ok(SyscallResult::Return(handle_id.as_usize()))
}

/// Allocates a vmarea at arbitrary physically-contiguous pages.
///
/// Similar to Linux's `dma_alloc_coherent` or NetBSD's `bus_dmamem_alloc`.
/// This is useful for device drivers that need DMA buffers with known
/// physical addresses.
pub fn sys_vmarea_alloc_contiguous(
    current: &SharedRef<Thread>,
    a0: usize,
    a1: usize,
) -> Result<SyscallResult, ErrorCode> {
    let size = a0;
    let paddr_out_ptr = IsolationPtr::new(a1);

    // Validate size is page-aligned
    if !is_aligned(size, MIN_PAGE_SIZE) {
        return Err(ErrorCode::InvalidArgument);
    }

    if size == 0 {
        return Err(ErrorCode::InvalidArgument);
    }

    // Allocate physically contiguous pages
    let paddr = PAGE_ALLOCATOR.allocate_pages_zeroed(size)?;

    // Create a pinned vmarea backed by this physical memory
    let vmarea = VmArea::new_pinned(paddr, size);
    let handle = Handle::new(SharedRef::new(vmarea)?, HandleRight::WRITE);
    let handle_id = current.process().handles().lock().insert(handle)?;

    // Write the physical address back to userspace
    let paddr_out = IsolationSliceMut::new(paddr_out_ptr, size_of::<usize>());
    paddr_out.write(current.process().isolation(), 0, paddr.as_usize())?;

    Ok(SyscallResult::Return(handle_id.as_usize()))
}
