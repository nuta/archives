use alloc::collections::BTreeMap;
use alloc::collections::btree_map;

use ftl_types::error::ErrorCode;
use ftl_types::handle::HandleId;

use crate::handle::AnyHandle;
use crate::handle::Handle;
use crate::handle::Handleable;
use crate::isolation::INKERNEL_ISOLATION;
use crate::isolation::Isolation;
use crate::shared_ref::SharedRef;
use crate::spinlock::SpinLock;
use crate::syscall::SyscallResult;
use crate::thread::Thread;
use crate::thread::sys_thread_exit;

const NUM_HANDLES_MAX: usize = 1024;

/// A process object.
pub struct Process {
    isolation: SharedRef<dyn Isolation>,
    handles: SpinLock<HandleTable>,
}

impl Process {
    pub fn create(isolation: SharedRef<dyn Isolation>) -> Self {
        Self {
            isolation,
            handles: SpinLock::new(HandleTable::new()),
        }
    }

    pub fn isolation(&self) -> &dyn Isolation {
        &*self.isolation
    }

    pub fn handles(&self) -> &SpinLock<HandleTable> {
        &self.handles
    }
}

impl Handleable for Process {}

/// The handles table for a process.
pub struct HandleTable {
    handles: BTreeMap<HandleId, AnyHandle>,
    next_id: i32,
}

impl HandleTable {
    pub fn new() -> Self {
        Self {
            handles: BTreeMap::new(),
            next_id: 1,
        }
    }

    pub fn insert<H: Into<AnyHandle>>(&mut self, object: H) -> Result<HandleId, ErrorCode> {
        if self.handles.len() >= NUM_HANDLES_MAX {
            return Err(ErrorCode::TooManyHandles);
        }

        let value = object.into();
        loop {
            let id = HandleId::from_raw(self.next_id);
            if id.as_usize() >= NUM_HANDLES_MAX {
                self.next_id = 1;
                continue;
            }

            self.next_id += 1;

            if let btree_map::Entry::Vacant(e) = self.handles.entry(id) {
                e.insert(value);
                return Ok(id);
            }
        }
    }

    pub fn insert_two<H: Into<AnyHandle>>(
        &mut self,
        object0: H,
        object1: H,
    ) -> Result<HandleId, ErrorCode> {
        let value0 = object0.into();
        let value1 = object1.into();

        loop {
            let id0 = HandleId::from_raw(self.next_id);
            let id1 = HandleId::from_raw(self.next_id + 1);
            if id1.as_usize() >= NUM_HANDLES_MAX {
                self.next_id = 1;
                continue;
            }

            self.next_id += 2;

            if !self.handles.contains_key(&id0) && !self.handles.contains_key(&id1) {
                self.handles.insert(id0, value0);
                self.handles.insert(id1, value1);
                return Ok(id0);
            }
        }
    }

    /// Returns the `AnyHandle` for the given handle id.
    pub fn get_any(&self, handle: HandleId) -> Result<AnyHandle, ErrorCode> {
        self.handles
            .get(&handle)
            .cloned()
            .ok_or(ErrorCode::HandleNotFound)
    }

    /// Returns a typed handle for the given handle id.
    ///
    /// If the handle is not the expected type, returns [`ErrorCode::HandleTypeMismatch`].
    pub fn get<T: Handleable>(&self, handle: HandleId) -> Result<Handle<T>, ErrorCode> {
        let any_handle = self.get_any(handle)?;
        let handle = any_handle.downcast().ok_or(ErrorCode::HandleTypeMismatch)?;
        Ok(handle)
    }

    /// Checks if the handle is movable.
    pub fn is_movable(&self, id: HandleId) -> bool {
        matches!(self.handles.get(&id), Some(handle) if handle.is_movable())
    }

    pub fn take(&mut self, handle: HandleId) -> Result<AnyHandle, ErrorCode> {
        let handle = self
            .handles
            .remove(&handle)
            .ok_or(ErrorCode::HandleNotFound)?;

        Ok(handle)
    }

    pub fn close(&mut self, handle: HandleId) -> Result<(), ErrorCode> {
        let handle = self.take(handle)?;
        handle.close();
        Ok(())
    }

    pub fn close_all(&mut self) {
        for handle in self.handles.values() {
            handle.close();
        }
        self.handles.clear();
    }
}

pub static IDLE_PROCESS: spin::Lazy<SharedRef<Process>> = spin::Lazy::new(|| {
    let process = Process::create(INKERNEL_ISOLATION.clone());
    SharedRef::new(process).unwrap()
});

pub fn sys_handle_close(
    current: &SharedRef<Thread>,
    id_raw: usize,
) -> Result<SyscallResult, ErrorCode> {
    let handle_id = HandleId::from_raw(id_raw as i32);
    let process = current.process();
    let mut handle_table = process.handles().lock();

    handle_table.close(handle_id)?;
    Ok(SyscallResult::Return(0))
}

pub fn sys_process_exit(current: &SharedRef<Thread>) -> Result<SyscallResult, ErrorCode> {
    // Close all handles and exit the current thread.
    current.process().handles().lock().close_all();
    sys_thread_exit(current)
}
