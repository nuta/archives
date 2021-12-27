use crate::utils::interface_wrapper::{construct_interface_wrapper, InterfaceWrapper};

use crate::wl::{
    Connection, DeserializeError, Handle, Interface, Message, NewId, ObjectId, Opcode, RawMessage,
    SendError,
};

use alloc::boxed::Box;
use alloc::rc::Rc;

use core::cell::{Ref, RefCell, RefMut};

pub mod wl_buffer;
pub mod wl_callback;
pub mod wl_compositor;
pub mod wl_display;
pub mod wl_keyboard;
pub mod wl_registry;
pub mod wl_seat;
pub mod wl_shm;
pub mod wl_shm_pool;
pub mod wl_surface;
pub mod xdg_surface;
pub mod xdg_toplevel;
pub mod xdg_wm_base;

use wl_buffer::BufferObject;
use wl_callback::CallbackObject;
use wl_compositor::CompositorObject;
use wl_display::DisplayObject;
use wl_keyboard::KeyboardObject;
use wl_registry::RegistryObject;
use wl_seat::SeatObject;
use wl_shm::ShmObject;
use wl_shm_pool::ShmPoolObject;
use wl_surface::SurfaceObject;
use xdg_surface::XdgSurfaceObject;
use xdg_toplevel::XdgToplevelObject;
use xdg_wm_base::XdgWmBaseObject;

/// The "unique numeric name of the global object". It should be in a separate
/// space from object IDs.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub struct GlobalObjectId(pub u32);

pub enum ObjectInner {
    Display(Rc<RefCell<DisplayObject>>),
    Registry(Rc<RefCell<RegistryObject>>),
    Seat(Rc<RefCell<SeatObject>>),
    Shm(Rc<RefCell<ShmObject>>),
    XdgWmBase(Rc<RefCell<XdgWmBaseObject>>),
    Compositor(Rc<RefCell<CompositorObject>>),
    Callback(Rc<RefCell<CallbackObject>>),
    ShmPool(Rc<RefCell<ShmPoolObject>>),
    Buffer(Rc<RefCell<BufferObject>>),
    Surface(Rc<RefCell<SurfaceObject>>),
    Keyboard(Rc<RefCell<KeyboardObject>>),
    XdgSurface(Rc<RefCell<XdgSurfaceObject>>),
    XdgToplevel(Rc<RefCell<XdgToplevelObject>>),
}

pub struct Object {
    id: ObjectId,
    interface: Box<dyn InterfaceWrapper>,
    inner: ObjectInner,
}

impl Object {
    pub fn new<I: Interface>(id: NewId, inner: ObjectInner) -> Object {
        Object {
            id: id.as_id(),
            interface: construct_interface_wrapper::<I>(),
            inner,
        }
    }

    pub fn id(&self) -> ObjectId {
        self.id
    }

    pub fn interface(&self) -> &dyn InterfaceWrapper {
        &*self.interface
    }

    pub fn inner(&self) -> &ObjectInner {
        &self.inner
    }

    pub fn inner_mut(&mut self) -> &mut ObjectInner {
        &mut self.inner
    }
}
