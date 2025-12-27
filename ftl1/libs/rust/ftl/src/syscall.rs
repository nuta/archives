use ftl_types::error::ErrorCode;

use crate::arch::get_vsyscall_page;

pub(crate) fn syscall0(n: usize) -> Result<usize, ErrorCode> {
    syscall(n, 0, 0, 0, 0, 0, 0)
}

pub(crate) fn syscall1(n: usize, a0: usize) -> Result<usize, ErrorCode> {
    syscall(n, a0, 0, 0, 0, 0, 0)
}

pub(crate) fn syscall2(n: usize, a0: usize, a1: usize) -> Result<usize, ErrorCode> {
    syscall(n, a0, a1, 0, 0, 0, 0)
}

pub(crate) fn syscall3(n: usize, a0: usize, a1: usize, a2: usize) -> Result<usize, ErrorCode> {
    syscall(n, a0, a1, a2, 0, 0, 0)
}

pub(crate) fn syscall4(
    n: usize,
    a0: usize,
    a1: usize,
    a2: usize,
    a3: usize,
) -> Result<usize, ErrorCode> {
    syscall(n, a0, a1, a2, a3, 0, 0)
}

pub(crate) fn syscall6(
    n: usize,
    a0: usize,
    a1: usize,
    a2: usize,
    a3: usize,
    a4: usize,
    a5: usize,
) -> Result<usize, ErrorCode> {
    syscall(n, a0, a1, a2, a3, a4, a5)
}

pub(crate) fn syscall(
    n: usize,
    a0: usize,
    a1: usize,
    a2: usize,
    a3: usize,
    a4: usize,
    a5: usize,
) -> Result<usize, ErrorCode> {
    let page = get_vsyscall_page();
    let (error, value) = (page.syscall_handler)(a0, a1, a2, a3, a4, a5, n); // "n" comes last
    if error != 0 {
        Err(ErrorCode::from(error as u8))
    } else {
        Ok(value)
    }
}
