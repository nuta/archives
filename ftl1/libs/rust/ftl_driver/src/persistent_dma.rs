use core::mem::size_of;
use core::ops::Deref;
use core::ops::DerefMut;

use ftl::error::ErrorCode;
use ftl::vmarea::VmArea;
use ftl::vmspace::CURRENT_VMSPACE;
use ftl_types::arch::PageAttrs;
use ftl_utils::alignment::align_up;

use crate::barrier::read_barrier;
use crate::barrier::write_barrier;
use crate::device::BusAddr;
use crate::device::DmaDesc;

#[derive(Debug)]
pub enum Error {
    VmAreaAlloc(ErrorCode),
    VmSpaceMap(ErrorCode),
}

/// Persistent DMA buffer, so-called *coherent DMA*.
pub struct PersistentDma<T> {
    baddr: BusAddr,
    ptr: *mut T,
    _vmarea: VmArea, // Keep vmarea alive to maintain the mapping
}

impl<T> PersistentDma<T> {
    /// Allocates a physically-contiguous DMA buffer.
    pub fn alloc(desc: DmaDesc) -> Result<Self, Error> {
        match desc {
            DmaDesc::Direct => {
                let size = align_up(size_of::<T>(), 0x1000); // TODO: use MIN_PAGE_SIZE
                let (vmarea, paddr) = VmArea::alloc_contiguous(size).map_err(Error::VmAreaAlloc)?;
                let attrs = PageAttrs::valid().read().write();
                let vaddr = CURRENT_VMSPACE
                    .map(&vmarea, 0, 0, size, attrs)
                    .map_err(Error::VmSpaceMap)?;
                let baddr = BusAddr::new(paddr);
                Ok(Self {
                    baddr,
                    ptr: vaddr as *mut T,
                    _vmarea: vmarea,
                })
            }
        }
    }

    pub fn baddr(&self) -> BusAddr {
        self.baddr
    }

    pub fn read_guard(&self) -> ReadGuard<'_, T> {
        self.before_read();
        ReadGuard::new(self)
    }

    pub fn write_guard(&mut self) -> WriteGuard<'_, T> {
        // Since &mut in Rust is also readable, we need to do the same thing
        // as read() here.
        self.before_read();
        self.before_write();
        WriteGuard::new(self)
    }

    fn before_read(&self) {
        read_barrier();
    }

    fn before_write(&self) {
        // No-op
    }

    fn after_read(&self) {
        // No-op
    }

    fn after_write(&self) {
        write_barrier();
    }
}

unsafe impl<T: Send> Send for PersistentDma<T> {}

pub struct ReadGuard<'a, T> {
    this: &'a PersistentDma<T>,
}

impl<'a, T> ReadGuard<'a, T> {
    fn new(this: &'a PersistentDma<T>) -> Self {
        Self { this }
    }

    pub fn invalidate_reads(&self) {
        self.this.before_read();
    }
}

impl<'a, T> Deref for ReadGuard<'a, T> {
    type Target = T;

    fn deref(&self) -> &Self::Target {
        unsafe { &*self.this.ptr }
    }
}

impl<'a, T> Drop for ReadGuard<'a, T> {
    fn drop(&mut self) {
        self.this.after_read();
    }
}

pub struct WriteGuard<'a, T> {
    this: &'a PersistentDma<T>,
}

impl<'a, T> WriteGuard<'a, T> {
    fn new(this: &'a PersistentDma<T>) -> Self {
        Self { this }
    }

    pub fn flush_writes(&self) {
        self.this.after_write();
    }
}

impl<'a, T> Deref for WriteGuard<'a, T> {
    type Target = T;

    fn deref(&self) -> &Self::Target {
        unsafe { &*self.this.ptr }
    }
}

impl<'a, T> DerefMut for WriteGuard<'a, T> {
    fn deref_mut(&mut self) -> &mut Self::Target {
        unsafe { &mut *self.this.ptr }
    }
}

impl<'a, T> Drop for WriteGuard<'a, T> {
    fn drop(&mut self) {
        self.this.after_write();
    }
}
