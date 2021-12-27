use crate::server::client::Client;

use crate::server::objects::xdg_surface::XdgSurfaceObject;

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
use alloc::string::String;
use core::cell::{Ref, RefCell, RefMut};

pub struct XdgToplevelObject {
    pub xdg_surface: Rc<RefCell<XdgSurfaceObject>>,
    pub title: String,
}

pub fn handle_request(
    client: &mut Client,
    object_id: ObjectId,
    req: xdg_toplevel::Request,
) -> Result<(), ServerError> {
    match req {
        xdg_toplevel::Request::SetTitle { title } => {
            let mut top_level = client.xdg_toplevel_object_mut(object_id)?;
            top_level.title = title;
        }
        req => {
            warn!("unhandled request: {:?}", req);
        }
    }

    Ok(())
}
