use crate::server::client::Client;

use crate::server::objects::{GlobalObjectId, Object, ObjectInner};
use crate::server::ServerError;
use crate::utils::interface_wrapper::InterfaceWrapper;

use crate::server::objects::{
    wl_compositor::CompositorObject, wl_shm::ShmObject, xdg_wm_base::XdgWmBaseObject,
};

use crate::wl::{
    protocols::common::{
        wl_buffer::{self, WlBuffer},
        wl_callback::{self, WlCallback},
        wl_compositor::{self, WlCompositor},
        wl_display::{self, WlDisplay},
        wl_keyboard::{self, WlKeyboard},
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

pub struct KeyboardObject {}

pub fn handle_request(
    client: &mut Client,
    _object_id: ObjectId,
    _req: wl_keyboard::Request,
) -> Result<(), ServerError> {
    Ok(())
}
