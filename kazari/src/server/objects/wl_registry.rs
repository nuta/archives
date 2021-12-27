use crate::server::client::Client;

use crate::server::objects::{GlobalObjectId, Object, ObjectInner};
use crate::server::ServerError;
use crate::utils::interface_wrapper::InterfaceWrapper;

use crate::server::objects::{
    wl_compositor::CompositorObject, wl_seat::SeatObject, wl_shm::ShmObject,
    xdg_wm_base::XdgWmBaseObject,
};

use crate::wl::{
    protocols::common::{
        wl_buffer::{self, WlBuffer},
        wl_callback::{self, WlCallback},
        wl_compositor::{self, WlCompositor},
        wl_display::{self, WlDisplay},
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
use alloc::boxed::Box;
use alloc::rc::Rc;
use alloc::string::ToString;
use core::cell::RefCell;
use hashbrown::HashMap;

pub struct RegistryObject {}

pub fn handle_request(
    client: &mut Client,
    _object_id: ObjectId,
    req: wl_registry::Request,
    global_objects: &HashMap<GlobalObjectId, Box<dyn InterfaceWrapper>>,
) -> Result<(), ServerError> {
    match req {
        wl_registry::Request::Bind { id, name } => {
            let interface = global_objects
                .get(&GlobalObjectId(name))
                .ok_or(ServerError::UnknownGlobal)?;
            match interface.name() {
                WlShm::NAME => {
                    client.register_object(Object::new::<WlShm>(
                        id,
                        ObjectInner::Shm(Rc::new(RefCell::new(ShmObject {}))),
                    ));
                }
                WlSeat::NAME => {
                    client.register_object(Object::new::<WlSeat>(
                        id,
                        ObjectInner::Seat(Rc::new(RefCell::new(SeatObject {}))),
                    ));
                }
                WlCompositor::NAME => {
                    client.register_object(Object::new::<WlCompositor>(
                        id,
                        ObjectInner::Compositor(Rc::new(RefCell::new(CompositorObject {}))),
                    ));
                }
                XdgWmBase::NAME => {
                    client.register_object(Object::new::<XdgWmBase>(
                        id,
                        ObjectInner::XdgWmBase(Rc::new(RefCell::new(XdgWmBaseObject {}))),
                    ));
                }
                _ => unreachable!(),
            }
        }
    }

    Ok(())
}
