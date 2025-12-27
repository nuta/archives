use alloc::boxed::Box;

use ftl_types::environ::VSyscallPage;
use ftl_types::error::ErrorCode;
use ftl_types::handle::HandleId;
use ftl_types::syscall::SYS_THREAD_EXIT;
use ftl_types::syscall::SYS_THREAD_SPAWN;

use crate::arch::get_vsyscall_page;
use crate::arch::set_vsyscall_page;
use crate::handle::Handleable;
use crate::handle::OwnedHandle;
use crate::process::CURRENT_PROCESS;
use crate::syscall::syscall0;
use crate::syscall::syscall4;

extern "C" fn thread_entry(arg: usize) -> ! {
    let closure = unsafe { Box::from_raw(arg as *mut Box<dyn FnOnce()>) };
    closure();
    unreachable!();
}

pub(crate) fn sys_thread_exit() -> ! {
    syscall0(SYS_THREAD_EXIT).unwrap();
    unreachable!();
}

#[derive(Debug)]
pub struct Thread {
    handle: OwnedHandle,
}

impl Thread {
    pub fn spawn<F>(f: F) -> Result<Self, ErrorCode>
    where
        F: FnOnce() + Send + 'static,
    {
        let vsyscall = get_vsyscall_page() as *const VSyscallPage;
        let closure: Box<dyn FnOnce()> = Box::new(move || {
            set_vsyscall_page(vsyscall);
            f();
            sys_thread_exit();
        });

        let process = CURRENT_PROCESS.handle_id().as_usize();
        let arg = Box::into_raw(Box::new(closure)) as usize;
        let pc = thread_entry as *const () as usize;
        let sp_bottom = alloc::vec![0u8; 32 * 1024].into_boxed_slice();
        let sp = (sp_bottom.as_ptr() as usize + sp_bottom.len()) & !0xf; // Align down to 16 bytes
        Box::leak(sp_bottom);

        // FIXME: If this fails, drop the closure to avoid a memory leak.
        let ret = syscall4(SYS_THREAD_SPAWN, process, pc, sp, arg)?;
        let handle = OwnedHandle::from_raw(HandleId::from_raw(ret as i32));
        Ok(Self { handle })
    }
}

impl Handleable for Thread {
    fn handle_id(&self) -> HandleId {
        self.handle.id()
    }
}
