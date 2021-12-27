use crate::server::client::Client;
use crate::server::objects::wl_buffer::BufferObject;

use crate::server::{ClientId, ServerError};

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
use crate::RectArea;
use crate::RectSize;

use alloc::rc::Rc;

use core::cell::{Ref, RefCell, RefMut};

pub struct SurfaceObject {
    pub buffer: Option<Rc<RefCell<BufferObject>>>,
    pub pos: Position,
    pub client_id: ClientId,
    pub object_id: ObjectId,
}

pub struct Changes {
    pub damaged_area: Option<RectArea>,
}

pub fn handle_request(
    client: &mut Client,
    object_id: ObjectId,
    req: wl_surface::Request,
) -> Result<Changes, ServerError> {
    let changes = match req {
        wl_surface::Request::Attach {
            buffer: new_buffer,
            x,
            y,
        } => {
            let new_buffer = match new_buffer {
                Some(new_buffer) => Some(client.buffer_object_rc(new_buffer.id())?.clone()),
                None => None,
            };

            let mut surface = client.surface_object_mut(object_id)?;
            surface.buffer = new_buffer;
            surface.pos = Position::new(x as usize, y as usize);
            Changes { damaged_area: None }
        }
        wl_surface::Request::Damage {
            x,
            y,
            width,
            height,
        } => {
            let area = RectArea::new(
                Position::new(x as usize, y as usize),
                RectSize::new(width as usize, height as usize),
            );
            Changes {
                damaged_area: Some(area),
            }
        }
        req => {
            warn!("unhandled request: {:?}", req);
            Changes { damaged_area: None }
        }
    };

    Ok(changes)
}
