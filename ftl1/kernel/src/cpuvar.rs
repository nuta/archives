use crate::arch;
use crate::shared_ref::SharedRef;
use crate::thread::CurrentThread;
use crate::thread::Thread;

#[derive(Clone, Copy, PartialEq, Eq)]
pub struct CpuId(usize);

impl CpuId {
    pub const fn new(id: usize) -> Self {
        Self(id)
    }

    pub const fn as_usize(self) -> usize {
        self.0
    }
}

/// Per-CPU variables.
///
/// It's `#[repr(C)]` to guarantee the arch's `CpuVar` comes first and the
/// addresses of both `arch::CpuVar` and this `CpuVar` are the same for
/// convenience.
#[repr(C)]
pub struct CpuVar {
    pub id: CpuId,
    pub arch: arch::CpuVar,
    pub current_thread: CurrentThread,
    pub idle_thread: SharedRef<Thread>,
}

/// Initializes CPU-local variables.
///
/// This function must be called for each CPU.
pub fn init(id: CpuId) {
    let idle_thread = Thread::new_idle().unwrap();
    arch::init_cpuvar(CpuVar {
        id,
        arch: arch::CpuVar::new(),
        current_thread: CurrentThread::new(&idle_thread),
        idle_thread,
    });
}
