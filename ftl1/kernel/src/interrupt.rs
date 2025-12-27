use alloc::collections::BTreeMap;

use ftl_types::error::ErrorCode;
use ftl_types::handle::HandleId;
use ftl_types::handle::HandleRight;
use ftl_types::poll::Readiness;

use crate::arch::INTERRUPT_CONTROLLER;
use crate::handle::Handle;
use crate::handle::Handleable;
use crate::poll::PollEmitter;
use crate::shared_ref::SharedRef;
use crate::spinlock::SpinLock;
use crate::syscall::SyscallResult;
use crate::thread::Thread;

struct Active {
    emitter: SharedRef<PollEmitter>,
}

static ACTIVE_INTERRUPTS: SpinLock<BTreeMap<usize, Active>> = SpinLock::new(BTreeMap::new());

pub struct Interrupt {
    irq: usize,
    emitter: SharedRef<PollEmitter>,
}

impl Interrupt {
    pub fn acquire(irq: usize) -> Result<Self, ErrorCode> {
        let mut actives = ACTIVE_INTERRUPTS.lock();
        if actives.contains_key(&irq) {
            return Err(ErrorCode::AlreadyExists);
        }

        INTERRUPT_CONTROLLER.acquire(irq)?;

        let emitter = PollEmitter::new(Readiness::empty());
        let emitter = SharedRef::new(emitter)?;
        let active = Active {
            emitter: emitter.clone(),
        };
        actives.insert(irq, active);

        Ok(Self { irq, emitter })
    }

    pub fn acknowledge(&self) {
        INTERRUPT_CONTROLLER.acknowledge(self.irq);
        self.emitter.clear(Readiness::READABLE);
    }
}

impl Drop for Interrupt {
    fn drop(&mut self) {
        let mut actives = ACTIVE_INTERRUPTS.lock();
        actives.remove(&self.irq);
        INTERRUPT_CONTROLLER.release(self.irq);
    }
}

impl Handleable for Interrupt {
    fn emitter(&self) -> Option<&PollEmitter> {
        Some(&self.emitter)
    }
}

pub fn sys_interrupt_acquire(
    current: &SharedRef<Thread>,
    a0: usize,
) -> Result<SyscallResult, ErrorCode> {
    let irq = a0;

    let irq = Interrupt::acquire(irq)?;
    let handle = Handle::new(SharedRef::new(irq)?, HandleRight::ALL);

    let handle_id = current.process().handles().lock().insert(handle)?;

    Ok(SyscallResult::Return(handle_id.as_usize()))
}

pub fn sys_interrupt_acknowledge(
    current: &SharedRef<Thread>,
    a0: usize,
) -> Result<SyscallResult, ErrorCode> {
    let handle_id = HandleId::from_usize(a0)?;

    let handles = current.process().handles().lock();
    let interrupt = handles
        .get::<Interrupt>(handle_id)?
        .authorize(HandleRight::WRITE)?;

    interrupt.acknowledge();

    Ok(SyscallResult::Return(0))
}

pub fn handle_irq(irq: usize) {
    let actives = ACTIVE_INTERRUPTS.lock();
    if let Some(active) = actives.get(&irq) {
        active.emitter.set(Readiness::READABLE);
    } else {
        trace!("spurious interrupt: {}", irq);
    }
}
