use crate::server::client::Client;

use crate::server::objects::wl_surface::SurfaceObject;

use crate::server::objects::{Object, ObjectInner};
use crate::server::ServerError;

use crate::wl::{
    protocols::common::{
        wl_buffer::{self, WlBuffer},
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
use crate::Position;

use alloc::rc::Rc;

use core::cell::{Ref, RefCell, RefMut};

pub struct CompositorObject {}

pub fn handle_request(
    client: &mut Client,
    _object_id: ObjectId,
    req: wl_compositor::Request,
) -> Result<(), ServerError> {
    match req {
        wl_compositor::Request::CreateSurface { id } => {
            client.register_object(Object::new::<WlSurface>(
                id,
                ObjectInner::Surface(Rc::new(RefCell::new(SurfaceObject {
                    client_id: client.client_id(),
                    object_id: id.as_id(),
                    buffer: None,
                    pos: Position::new(0, 0),
                }))),
            ));
        }
        req => {
            warn!("unhandled request: {:?}", req);
        }
    }

    Ok(())
}
