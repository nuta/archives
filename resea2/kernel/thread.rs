use crate::{
    arch::{switch_thread, ArchThread, TryAsVAddr},
    memory::alloc_pages,
};
use arch_common::vaddr::VAddr;
use etc::addr_of_fn;

pub struct Thread {
    arch: ArchThread,
}

impl Thread {
    pub fn new_kernel_thread(entry: VAddr) -> Thread {
        let stack_top = alloc_pages(16).unwrap().try_as_vaddr().unwrap();
        Thread {
            arch: ArchThread::new_kernel_thread(entry, stack_top),
        }
    }

    pub fn new_idle_thread() -> Thread {
        Thread {
            arch: ArchThread::new_idle_thread(),
        }
    }

    pub fn arch(&self) -> &ArchThread {
        &self.arch
    }
}

static IDLE_THREAD: spin::Once<Thread> = spin::Once::new();
static THREAD_A: spin::Once<Thread> = spin::Once::new();

fn thread_a() {
    let idle_thread = IDLE_THREAD.get().expect("not initialized");
    let thread_a = THREAD_A.get().expect("not initialized");

    loop {
        println!("Hello from thread A!");
        switch_thread(thread_a.arch(), idle_thread.arch());
    }
}

pub fn init() {
    let thread_a_entry = unsafe { VAddr::new(addr_of_fn!(thread_a)) };
    THREAD_A.call_once(|| Thread::new_kernel_thread(thread_a_entry));
    IDLE_THREAD.call_once(|| Thread::new_idle_thread());

    let idle_thread = IDLE_THREAD.get().expect("not initialized");
    let thread_a = THREAD_A.get().expect("not initialized");

    for _ in 0..16 {
        println!("Hello from idle thread!");
        switch_thread(idle_thread.arch(), thread_a.arch());
    }

    panic!("OK!");
}
