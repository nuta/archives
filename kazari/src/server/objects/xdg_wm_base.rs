use crate::server::client::Client;

use crate::server::objects::xdg_surface::XdgSurfaceObject;

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

use alloc::rc::Rc;

use core::cell::{Ref, RefCell, RefMut};

pub struct XdgWmBaseObject {}

pub fn handle_request(
    client: &mut Client,
    _object_id: ObjectId,
    req: xdg_wm_base::Request,
) -> Result<(), ServerError> {
    match req {
        xdg_wm_base::Request::GetXdgSurface { id, surface } => {
            client.register_object(Object::new::<XdgSurface>(
                id,
                ObjectInner::XdgSurface(Rc::new(RefCell::new(XdgSurfaceObject {
                    surface: client.surface_object_rc(surface.id())?.clone(),
                }))),
            ));
        }
        req => {
            warn!("unhandled request: {:?}", req);
        }
    }

    Ok(())
}
