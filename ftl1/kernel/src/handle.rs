use core::any::Any;
use core::ops::Deref;

use ftl_types::error::ErrorCode;
use ftl_types::handle::HandleRight;

use crate::poll::PollEmitter;
use crate::shared_ref::SharedRef;

/// Handle, a reference-counted pointer to a kernel object with allowed
/// operations on it, aka *"capability"*.
pub struct Handle<T: Handleable + ?Sized> {
    object: SharedRef<T>,
    rights: HandleRight,
}

impl<T: Handleable + ?Sized> Handle<T> {
    pub fn new(object: SharedRef<T>, rights: HandleRight) -> Self {
        Self { object, rights }
    }

    /// Returns a reference to the underlying object.
    ///
    /// It takes rights as an argument to check if the handle is allowed to
    /// do the following operations.
    pub fn authorize(self, required: HandleRight) -> Result<SharedRef<T>, ErrorCode> {
        if self.rights.contains(required) {
            Ok(self.object)
        } else {
            Err(ErrorCode::NotAllowed)
        }
    }
}

impl<T: Handleable + ?Sized> Clone for Handle<T> {
    fn clone(&self) -> Self {
        Handle {
            object: self.object.clone(),
            rights: self.rights,
        }
    }
}

/// A trait for kernel objects that can be referenced by a handle.
pub trait Handleable: Any + Send + Sync {
    /// Closes the object.
    ///
    /// Note that the object might be referenced by other handles.
    fn close(&self) {
        // Do nothing by default.
    }

    /// Returns a `PollEmitter` if the object can be polled.
    fn emitter(&self) -> Option<&PollEmitter> {
        // Not pollable by default.
        None
    }

    /// Returns whether the object can be moved between processes.
    fn is_movable(&self) -> bool {
        // Not movable by default.
        false
    }

    /// Returns whether the object emits edge-triggered events.
    ///
    /// If `true`, the emitter will be cleared after the event is read
    /// by a poll.
    fn is_edge_triggered(&self) -> bool {
        // Not edge-triggered by default.
        false
    }
}

#[derive(Clone)]
pub struct AnyHandle(Handle<dyn Handleable>);

impl AnyHandle {
    pub fn downcast<T: Handleable>(self) -> Option<Handle<T>> {
        let object = self.0.object.downcast().ok()?;
        let rights = self.0.rights;
        Some(Handle { object, rights })
    }
}

impl<T: Handleable> From<Handle<T>> for AnyHandle {
    fn from(handle: Handle<T>) -> Self {
        AnyHandle(Handle {
            object: handle.object,
            rights: handle.rights,
        })
    }
}

impl Deref for AnyHandle {
    type Target = dyn Handleable;

    fn deref(&self) -> &Self::Target {
        &*self.0.object
    }
}
