use core::slice;

pub type SyscallHandler = fn(usize, usize, usize, usize, usize, usize, usize) -> (usize, usize);

#[repr(C)]
pub struct VSyscallPage {
    /// A pointer to the syscall handler function. Returns (`error`, `value`),
    /// where `error` is 0 on success, or an error code (`ErrorCode`) on failure.
    pub syscall_handler: SyscallHandler,
    pub environ_ptr: usize,
    pub environ_len: usize,
    pub name_ptr: usize,
    pub name_len: usize,
}

impl VSyscallPage {
    pub fn name(&self) -> &str {
        unsafe {
            let name_bytes = slice::from_raw_parts(self.name_ptr as *const u8, self.name_len);
            core::str::from_utf8_unchecked(name_bytes)
        }
    }
}
