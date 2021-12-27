use crate::server::client::Client;

use crate::server::objects::{GlobalObjectId, Object, ObjectInner};
use crate::server::ServerError;
use crate::utils::interface_wrapper::InterfaceWrapper;

use crate::server::objects::{
    wl_compositor::CompositorObject, wl_shm::ShmObject, xdg_wm_base::XdgWmBaseObject,
};

use crate::server::objects::wl_keyboard::KeyboardObject;
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

pub struct SeatObject {}

pub fn handle_request(
    client: &mut Client,
    _object_id: ObjectId,
    req: wl_seat::Request,
) -> Result<(), ServerError> {
    match req {
        wl_seat::Request::GetKeyboard { id } => {
            let keyboard_object = Object::new::<WlKeyboard>(
                id,
                ObjectInner::Keyboard(Rc::new(RefCell::new(KeyboardObject {}))),
            );
            client.register_object(keyboard_object);
            client.set_keyboard_object(id.as_id());
        }
        _ => {
            warn!("unhandled request: {:?}", req);
        }
    }
    Ok(())
}
