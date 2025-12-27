mod inkernel;

use alloc::vec::Vec;
use core::mem::MaybeUninit;
use core::mem::size_of;
use core::slice;

use ftl_types::error::ErrorCode;
pub use inkernel::INKERNEL_ISOLATION;

use crate::shared_ref::SharedRef;
use crate::vmspace::VmSpace;

/// A process's memory isolation.
pub trait Isolation: Send + Sync {
    fn vmspace(&self) -> &SharedRef<VmSpace>;
    fn read_bytes(&self, ptr: IsolationPtr, dst: &mut [u8]) -> Result<(), ErrorCode>;
    fn write_bytes(&self, ptr: IsolationPtr, src: &[u8]) -> Result<(), ErrorCode>;

    /// Returns the pointer `base + offset`, after checking if
    /// `[base + offset, base + offset + len)` is in the range of `[base, base + max_len)`.
    fn is_accessible(
        &self,
        base: IsolationPtr,
        max_len: usize,
        offset: usize,
        len: usize,
    ) -> Result<IsolationPtr, ErrorCode> {
        // Check overflows.
        let start_ptr = base.0.checked_add(offset).ok_or(ErrorCode::OutOfBounds)?;
        let _end_ptr = start_ptr.checked_add(len).ok_or(ErrorCode::OutOfBounds)?;

        // Check if it's within the slice bounds.
        if offset + len > max_len {
            return Err(ErrorCode::OutOfBounds);
        }

        Ok(IsolationPtr::new(start_ptr))
    }
}

/// A pointer in an isolation space.
///
/// This is an opaque value and depends on the isolation implementation. For example,
/// it is a raw kernel pointer in the in-kernel isolation, a user pointer in the
/// user-space isolation, or a memory offset in WebAssembly isolation.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct IsolationPtr(usize);

impl IsolationPtr {
    pub const fn new(ptr: usize) -> Self {
        Self(ptr)
    }
}

/// A slice in an isolation space.
pub struct IsolationSlice {
    ptr: IsolationPtr,
    len: usize,
}

impl IsolationSlice {
    pub const fn new(ptr: IsolationPtr, len: usize) -> Self {
        Self { ptr, len }
    }

    pub const fn from_raw(raw_ptr: usize, len: usize) -> Self {
        let ptr = IsolationPtr::new(raw_ptr);
        Self::new(ptr, len)
    }

    pub fn len(&self) -> usize {
        self.len
    }

    pub fn read<T: Copy>(&self, isolation: &dyn Isolation, offset: usize) -> Result<T, ErrorCode> {
        let checked_ptr = isolation.is_accessible(self.ptr, self.len, offset, size_of::<T>())?;

        let mut buf = MaybeUninit::uninit();
        let buf_ptr = buf.as_mut_ptr() as *mut u8;
        let buf_slice = unsafe { slice::from_raw_parts_mut(buf_ptr, size_of::<T>()) };

        isolation.read_bytes(checked_ptr, buf_slice)?;
        Ok(unsafe { buf.assume_init() })
    }

    pub fn read_bytes(
        &self,
        isolation: &dyn Isolation,
        offset: usize,
        buf: &mut [u8],
    ) -> Result<(), ErrorCode> {
        let checked_ptr = isolation.is_accessible(self.ptr, self.len, offset, buf.len())?;
        isolation.read_bytes(checked_ptr, buf)
    }

    pub fn read_to_vec(
        &self,
        isolation: &dyn Isolation,
        offset: usize,
        len: usize,
    ) -> Result<Vec<u8>, ErrorCode> {
        let checked_ptr = isolation.is_accessible(self.ptr, self.len, offset, len)?;

        // A terrible hack to avoid zero-initializing the vector unnecessarily.
        let mut buf = Vec::with_capacity(len);
        #[allow(clippy::uninit_vec)]
        unsafe {
            buf.set_len(len);
        }

        isolation.read_bytes(checked_ptr, &mut buf)?;
        Ok(buf)
    }
}

pub struct IsolationSliceMut {
    slice: IsolationSlice,
}

impl IsolationSliceMut {
    pub const fn new(ptr: IsolationPtr, len: usize) -> Self {
        Self {
            slice: IsolationSlice::new(ptr, len),
        }
    }

    pub fn write<T: Copy>(
        &self,
        isolation: &dyn Isolation,
        offset: usize,
        value: T,
    ) -> Result<(), ErrorCode> {
        let checked_ptr =
            isolation.is_accessible(self.slice.ptr, self.slice.len, offset, size_of::<T>())?;
        let value_ptr = &value as *const T as *const u8;
        let value_bytes = unsafe { slice::from_raw_parts(value_ptr, size_of::<T>()) };
        isolation.write_bytes(checked_ptr, value_bytes)?;
        Ok(())
    }

    pub fn write_bytes(
        &self,
        isolation: &dyn Isolation,
        offset: usize,
        slice: &[u8],
    ) -> Result<(), ErrorCode> {
        let checked_ptr =
            isolation.is_accessible(self.slice.ptr, self.slice.len, offset, slice.len())?;
        isolation.write_bytes(checked_ptr, slice)
    }
}
