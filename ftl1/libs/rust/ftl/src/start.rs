use core::slice;

use ftl_types::environ::VSyscallPage;
use ftl_types::syscall::SYS_PROCESS_EXIT;

use crate::arch;
use crate::syscall::syscall0;

/// The entry point of an application.
///
/// This function is the first function that is called when an application
/// is started. It initializes the standard library, and calls the `main`
/// function.
#[allow(clippy::not_unsafe_ptr_arg_deref)]
pub fn start<E, F>(vsyscall: *const VSyscallPage, main: F) -> !
where
    E: serde::de::DeserializeOwned,
    F: Fn(E),
{
    arch::set_vsyscall_page(vsyscall);
    crate::log::init();

    // SAFETY: Trust the kernel to provide a valid vsyscall page.
    let env_json = unsafe {
        slice::from_raw_parts(
            (*vsyscall).environ_ptr as *const u8,
            (*vsyscall).environ_len,
        )
    };
    let env: E = serde_json::from_slice(env_json).expect("failed to parse environ as JSON");

    main(env);

    syscall0(SYS_PROCESS_EXIT).expect("failed to exit process");
    unreachable!();
}
