use crate::server::client::Client;

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
use alloc::string::ToString;
use alloc::vec::Vec;

use crate::server::layer::{Layer, LayerContent};
use crate::server::objects::wl_surface::SurfaceObject;
use crate::server::objects::xdg_toplevel::XdgToplevelObject;
use crate::server::objects::{Object, ObjectInner};
use crate::server::ServerError;
use crate::server::WINDOW_TITLE_BAR_HEIGHT;
use crate::Position;
use crate::RectSize;
use core::cell::{Ref, RefCell, RefMut};

pub struct XdgSurfaceObject {
    pub surface: Rc<RefCell<SurfaceObject>>,
}

pub fn handle_request(
    client: &mut Client,
    object_id: ObjectId,
    req: xdg_surface::Request,
    layers: &mut Vec<Layer>,
) -> Result<(), ServerError> {
    match req {
        xdg_surface::Request::GetToplevel { id } => {
            let xdg_surface = client.xdg_surface_object_rc(object_id)?.clone();
            let xdg_toplevel = Rc::new(RefCell::new(XdgToplevelObject {
                xdg_surface: xdg_surface.clone(),
                title: "New Window".to_string(),
            }));
            client.register_object(Object::new::<XdgToplevel>(
                id,
                ObjectInner::XdgToplevel(xdg_toplevel.clone()),
            ));

            let surface = xdg_surface.borrow().surface.clone();
            let size = surface
                .borrow()
                .buffer
                .as_ref()
                .map(|b| {
                    let size = b.borrow().canvas.size();
                    RectSize::new(size.width, size.height + WINDOW_TITLE_BAR_HEIGHT)
                })
                .unwrap_or(RectSize::new(0, WINDOW_TITLE_BAR_HEIGHT)); // FIXME:

            layers.push(Layer::new(
                Position::new(30, 30),
                size,
                LayerContent::Window {
                    surface,
                    xdg_toplevel,
                },
                true,
            ));
        }
        req => {
            warn!("unhandled request: {:?}", req);
        }
    }

    Ok(())
}
