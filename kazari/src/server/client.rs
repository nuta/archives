use crate::server::objects::wl_buffer::BufferObject;
use crate::server::objects::wl_display::DisplayObject;
use crate::server::objects::wl_shm_pool::ShmPoolObject;
use crate::server::objects::wl_surface::SurfaceObject;
use crate::server::objects::xdg_surface::XdgSurfaceObject;
use crate::server::objects::xdg_toplevel::XdgToplevelObject;
use crate::server::objects::{Object, ObjectInner};
use crate::server::ServerError;
use crate::utils::interface_wrapper::InterfaceWrapper;
use crate::wl::{
    protocols::common::{
        wl_buffer::{self, WlBuffer},
        wl_compositor::{self, WlCompositor},
        wl_display::{self, WlDisplay},
        wl_keyboard::WlKeyboard,
        wl_registry,
        wl_seat::{self, WlSeat},
        wl_shm::{self, WlShm},
        wl_shm_pool::{self, WlShmPool},
        wl_surface::{self, WlSurface},
        xdg_surface::{self, XdgSurface},
        xdg_toplevel::{self, XdgToplevel},
        xdg_wm_base::{self, XdgWmBase},
        RequestSet,
    },
    Connection, DeserializeError, Handle, Interface, Message, NewId, ObjectId, Opcode, RawMessage,
    SendError,
};
use alloc::rc::Rc;
use alloc::string::ToString;
use core::cell::{Ref, RefCell, RefMut};
use hashbrown::HashMap;

macro_rules! object_accessor {
    ($self:ident, $interface:ident, $inner:path, $id:expr) => {
        match $self.object::<$interface>($id)?.inner() {
            $inner(rc) => Ok(rc.borrow()),
            _ => unreachable!(),
        }
    };
}

macro_rules! object_accessor_mut {
    ($self:ident, $interface:ident, $inner:path, $id:expr) => {
        match $self.object_mut::<$interface>($id)?.inner() {
            $inner(rc) => Ok(rc.borrow_mut()),
            _ => unreachable!(),
        }
    };
}

macro_rules! object_rc_accessor {
    ($self:ident, $interface:ident, $inner:path, $id:expr) => {
        match $self.object::<$interface>($id)?.inner() {
            $inner(rc) => Ok(rc),
            _ => unreachable!(),
        }
    };
}

#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub struct ClientId(pub i32);

impl ClientId {
    pub const fn new(id: i32) -> ClientId {
        ClientId(id)
    }
}

pub struct Client {
    client_id: ClientId,
    connection: Rc<RefCell<dyn Connection>>,
    objects: HashMap<ObjectId, Object>,
    serial: u32,
    keyboard_object: Option<WlKeyboard>,
}

impl Client {
    pub fn new(client_id: ClientId, connection: Rc<RefCell<dyn Connection>>) -> Client {
        let mut objects = HashMap::new();
        let display = ObjectInner::Display(Rc::new(RefCell::new(DisplayObject {})));
        objects.insert(ObjectId(1), Object::new::<WlDisplay>(NewId(1), display));
        Client {
            client_id,
            connection,
            objects,
            serial: 1,
            keyboard_object: None,
        }
    }

    pub fn client_id(&self) -> ClientId {
        self.client_id
    }

    pub fn connection(&self) -> &Rc<RefCell<dyn Connection>> {
        &self.connection
    }

    pub fn send_event<E: Message>(&self, object: ObjectId, event: E) -> Result<(), ServerError> {
        self.connection
            .borrow_mut()
            .send(event.into_raw(object))
            .map_err(ServerError::SendError)
    }

    pub fn send_error(
        &self,
        object: ObjectId,
        code: u32,
        message: &str,
    ) -> Result<(), ServerError> {
        self.connection
            .borrow_mut()
            .send(
                wl_display::Event::Error {
                    code,
                    object_id: object,
                    message: message.to_string(),
                }
                .into_raw(ObjectId(1) /* wl_display */),
            )
            .map_err(ServerError::SendError)
    }

    pub fn alloc_serial(&mut self) -> u32 {
        let new_serial = self.serial;
        self.serial += 1;
        new_serial
    }

    pub fn register_object(&mut self, object: Object) {
        self.objects.insert(object.id(), object);
    }

    pub fn keyboard_object(&self) -> Option<&WlKeyboard> {
        self.keyboard_object.as_ref()
    }

    pub fn set_keyboard_object(&mut self, object: ObjectId) {
        self.keyboard_object = Some(WlKeyboard::new(self.connection().clone(), object));
    }

    pub fn object_interface(&self, object: ObjectId) -> Result<&dyn InterfaceWrapper, ServerError> {
        let object = self
            .objects
            .get(&object)
            .ok_or(ServerError::UnknownObjectId(object))?;
        Ok(object.interface())
    }

    pub fn shm_pool_object(
        &self,
        object_id: ObjectId,
    ) -> Result<Ref<'_, ShmPoolObject>, ServerError> {
        object_accessor!(self, WlShmPool, ObjectInner::ShmPool, object_id)
    }

    pub fn surface_object_mut(
        &mut self,
        object_id: ObjectId,
    ) -> Result<RefMut<'_, SurfaceObject>, ServerError> {
        object_accessor_mut!(self, WlSurface, ObjectInner::Surface, object_id)
    }

    pub fn xdg_toplevel_object_mut(
        &mut self,
        object_id: ObjectId,
    ) -> Result<RefMut<'_, XdgToplevelObject>, ServerError> {
        object_accessor_mut!(self, XdgToplevel, ObjectInner::XdgToplevel, object_id)
    }

    pub fn buffer_object_rc(
        &self,
        object_id: ObjectId,
    ) -> Result<&Rc<RefCell<BufferObject>>, ServerError> {
        object_rc_accessor!(self, WlBuffer, ObjectInner::Buffer, object_id)
    }

    pub fn surface_object_rc(
        &self,
        object_id: ObjectId,
    ) -> Result<&Rc<RefCell<SurfaceObject>>, ServerError> {
        object_rc_accessor!(self, WlSurface, ObjectInner::Surface, object_id)
    }

    pub fn xdg_surface_object_rc(
        &self,
        object_id: ObjectId,
    ) -> Result<&Rc<RefCell<XdgSurfaceObject>>, ServerError> {
        object_rc_accessor!(self, XdgSurface, ObjectInner::XdgSurface, object_id)
    }

    fn object<I: Interface>(&self, object: ObjectId) -> Result<&Object, ServerError> {
        self.objects
            .get(&object)
            .ok_or(ServerError::UnknownObjectId(object))
            .and_then(|object| {
                if object.interface().name() == I::NAME {
                    Ok(object)
                } else {
                    Err(ServerError::ObjectMismatch)
                }
            })
    }

    fn object_mut<I: Interface>(&mut self, object: ObjectId) -> Result<&mut Object, ServerError> {
        self.objects
            .get_mut(&object)
            .ok_or(ServerError::UnknownObjectId(object))
            .and_then(|object| {
                if object.interface().name() == I::NAME {
                    Ok(object)
                } else {
                    Err(ServerError::ObjectMismatch)
                }
            })
    }
}
