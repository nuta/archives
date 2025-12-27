use ftl_types::error::ErrorCode;
use ftl_types::syscall::SYS_CHANNEL_CREATE;
use ftl_types::syscall::SYS_CHANNEL_RECV;
use ftl_types::syscall::SYS_CHANNEL_SEND;
use ftl_types::syscall::SYS_HANDLE_CLOSE;
use ftl_types::syscall::SYS_INTERRUPT_ACKNOWLEDGE;
use ftl_types::syscall::SYS_INTERRUPT_ACQUIRE;
use ftl_types::syscall::SYS_LOG_WRITE;
use ftl_types::syscall::SYS_POLL_ADD;
use ftl_types::syscall::SYS_POLL_CREATE;
use ftl_types::syscall::SYS_POLL_REMOVE;
use ftl_types::syscall::SYS_POLL_UPDATE;
use ftl_types::syscall::SYS_POLL_WAIT;
use ftl_types::syscall::SYS_PROCESS_EXIT;
use ftl_types::syscall::SYS_THREAD_EXIT;
use ftl_types::syscall::SYS_THREAD_SPAWN;
use ftl_types::syscall::SYS_TIMER_CREATE;
use ftl_types::syscall::SYS_TIMER_NOW;
use ftl_types::syscall::SYS_TIMER_SET;
use ftl_types::syscall::SYS_VMAREA_ALLOC;
use ftl_types::syscall::SYS_VMAREA_ALLOC_CONTIGUOUS;
use ftl_types::syscall::SYS_VMAREA_PIN;
use ftl_types::syscall::SYS_VMSPACE_MAP;

use crate::arch::get_cpuvar;
use crate::shared_ref::SharedRef;
use crate::thread::Promise;
use crate::thread::Thread;
use crate::thread::return_to_user;

pub enum SyscallResult {
    Return(usize),
    Block(Promise),
    Exit,
}

#[allow(clippy::too_many_arguments)]
fn do_handle_syscall(
    current: &SharedRef<Thread>,
    a0: usize,
    a1: usize,
    a2: usize,
    a3: usize,
    a4: usize,
    a5: usize,
    n: usize,
) -> Result<SyscallResult, ErrorCode> {
    match n {
        SYS_HANDLE_CLOSE => crate::process::sys_handle_close(current, a0),
        SYS_LOG_WRITE => crate::print::sys_log_write(current, a0, a1),
        SYS_PROCESS_EXIT => crate::process::sys_process_exit(current),
        SYS_THREAD_SPAWN => crate::thread::sys_thread_spawn(current, a0, a1, a2, a3),
        SYS_THREAD_EXIT => crate::thread::sys_thread_exit(current),
        SYS_CHANNEL_CREATE => crate::channel::sys_channel_create(current),
        SYS_CHANNEL_SEND => crate::channel::sys_channel_send(current, a0, a1, a2, a3),
        SYS_CHANNEL_RECV => crate::channel::sys_channel_recv(current, a0, a1, a2),
        SYS_POLL_CREATE => crate::poll::sys_poll_create(current),
        SYS_POLL_ADD => crate::poll::sys_poll_add(current, a0, a1, a2),
        SYS_POLL_UPDATE => crate::poll::sys_poll_update(current, a0, a1, a2, a3),
        SYS_POLL_WAIT => crate::poll::sys_poll_wait(current, a0),
        SYS_POLL_REMOVE => crate::poll::sys_poll_remove(current, a0, a1),
        SYS_VMSPACE_MAP => crate::vmspace::sys_vmspace_map(current, a0, a1, a2, a3, a4, a5),
        SYS_VMAREA_ALLOC => crate::vmarea::sys_vmarea_alloc(current, a0),
        SYS_VMAREA_PIN => crate::vmarea::sys_vmarea_pin(current, a0, a1),
        SYS_VMAREA_ALLOC_CONTIGUOUS => crate::vmarea::sys_vmarea_alloc_contiguous(current, a0, a1),
        SYS_INTERRUPT_ACQUIRE => crate::interrupt::sys_interrupt_acquire(current, a0),
        SYS_INTERRUPT_ACKNOWLEDGE => crate::interrupt::sys_interrupt_acknowledge(current, a0),
        SYS_TIMER_NOW => crate::timer::sys_timer_now(),
        SYS_TIMER_CREATE => crate::timer::sys_timer_create(current),
        SYS_TIMER_SET => crate::timer::sys_timer_set(current, a0, a1),
        _ => {
            debug_warn!("unknown syscall: {}", n);
            Err(ErrorCode::UnknownSyscall)
        }
    }
}

pub fn handle_syscall(
    a0: usize,
    a1: usize,
    a2: usize,
    a3: usize,
    a4: usize,
    a5: usize,
    n: usize,
) -> ! {
    let current = &get_cpuvar().current_thread;
    let current_ref = current.as_sharedref();
    match do_handle_syscall(&current_ref, a0, a1, a2, a3, a4, a5, n) {
        Ok(output) => {
            match output {
                SyscallResult::Block(blocked_by) => {
                    current.block(blocked_by);
                }
                SyscallResult::Return(retval) => {
                    current.set_syscall_result(Ok(retval));
                }
                SyscallResult::Exit => {
                    // Do nothing.
                }
            }
        }
        Err(e) => {
            current.set_syscall_result(Err(e));
        }
    }

    return_to_user();
}
