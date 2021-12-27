use core::ptr::NonNull;

pub trait UIntLike {
    /// Returns `0`. Interestingly, you can build arbitrary integer `n` by using
    /// this method: `<T as UIntLike>::zero() + n`.
    ///
    /// This idea comes from tock-registers crate:
    /// https://docs.rs/tock-registers/0.7.0/tock_registers/trait.UIntLike.html
    fn zero() -> Self;
}

impl UIntLike for u8 {
    fn zero() -> Self {
        0
    }
}

impl UIntLike for u16 {
    fn zero() -> Self {
        0
    }
}

impl UIntLike for u32 {
    fn zero() -> Self {
        0
    }
}

impl UIntLike for u64 {
    fn zero() -> Self {
        0
    }
}

/// A read-only MMIO register.
pub struct ReadOnlyReg<T: UIntLike> {
    addr: NonNull<T>,
}

impl<T: UIntLike> ReadOnlyReg<T> {
    pub const fn new(addr: usize) -> ReadOnlyReg<T> {
        ReadOnlyReg {
            addr: unsafe { NonNull::new_unchecked(addr as *mut _) },
        }
    }
}

/// A write-only MMIO register.
pub struct WriteOnlyReg<T: UIntLike> {
    addr: NonNull<T>,
}

impl<T: UIntLike> WriteOnlyReg<T> {
    pub const fn new(addr: usize) -> WriteOnlyReg<T> {
        WriteOnlyReg {
            addr: unsafe { NonNull::new_unchecked(addr as *mut _) },
        }
    }
}

pub trait ReadableReg {
    type Target;
    unsafe fn read(&self) -> Self::Target;
}

impl<T: UIntLike> ReadableReg for ReadOnlyReg<T> {
    type Target = T;
    unsafe fn read(&self) -> Self::Target {
        self.addr.as_ptr().read_volatile()
    }
}

pub trait WritableReg {
    type Target;
    unsafe fn write(&self, value: Self::Target);
}

impl<T: UIntLike> WritableReg for WriteOnlyReg<T> {
    type Target = T;
    unsafe fn write(&self, value: Self::Target) {
        self.addr.as_ptr().write_volatile(value);
    }
}
