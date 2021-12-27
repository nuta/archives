use crate::canvas::CanvasBuffer;
use crate::wl::Handle;
use alloc::rc::Rc;

pub struct MMap {
    handle: Handle,
    buffer: *mut u8,
    len: usize,
    allocator: &'static dyn MMapAllocator,
}

#[allow(clippy::len_without_is_empty)]
impl MMap {
    pub fn new(
        allocator: &'static dyn MMapAllocator,
        handle: Handle,
        buffer: *mut u8,
        len: usize,
    ) -> MMap {
        MMap {
            handle,
            buffer,
            len,
            allocator,
        }
    }

    pub fn from_handle(
        allocator: &'static dyn MMapAllocator,
        handle: Handle,
        len: usize,
    ) -> Result<MMap, MMapError> {
        let (our_handle, buffer) = allocator.map(handle, len)?;
        Ok(MMap::new(allocator, our_handle, buffer, len))
    }

    pub fn handle(&self) -> Handle {
        self.handle
    }

    pub fn len(&self) -> usize {
        self.len
    }

    pub fn buffer(&self) -> *mut u8 {
        self.buffer
    }
}

impl CanvasBuffer for Rc<MMap> {
    fn as_ref(&self) -> &[u8] {
        unsafe { core::slice::from_raw_parts(self.buffer, self.len) }
    }

    fn as_mut(&mut self) -> &mut [u8] {
        unsafe { core::slice::from_raw_parts_mut(self.buffer, self.len) }
    }
}

impl Drop for MMap {
    fn drop(&mut self) {
        self.allocator.free(self.handle);
    }
}

#[derive(Debug, PartialEq)]
pub struct NoMemoryError;
#[derive(Debug, PartialEq)]
pub struct MMapError;

pub trait MMapAllocator {
    fn alloc(&self, len: usize) -> Result<(Handle, *mut u8), NoMemoryError>;
    fn map(&self, handle: Handle, len: usize) -> Result<(Handle, *mut u8), MMapError>;
    fn free(&self, mmap: Handle);
}
