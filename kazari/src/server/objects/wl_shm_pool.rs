use crate::canvas::Canvas;
use crate::server::client::Client;
use crate::server::objects::wl_buffer::BufferObject;

use crate::server::objects::{Object, ObjectInner};
use crate::server::ServerError;

use crate::utils::mmap::MMap;
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
use crate::PixelFormat;

use alloc::boxed::Box;
use alloc::rc::Rc;

use core::cell::{Ref, RefCell, RefMut};

pub struct ShmPoolObject {
    pub mmap: Rc<MMap>,
}

pub fn handle_request(
    client: &mut Client,
    object_id: ObjectId,
    req: wl_shm_pool::Request,
) -> Result<(), ServerError> {
    match req {
        wl_shm_pool::Request::CreateBuffer {
            id,
            offset: _,
            width,
            height,
            stride: _,
            format,
        } => {
            let format = match format {
                wl_shm::Format::Rgba8888 => PixelFormat::RGBA8888,
                _ => {
                    client.send_error(
                        object_id,
                        wl_shm::Error::InvalidFormat as u32,
                        "unsupported format",
                    )?;
                    return Ok(());
                }
            };

            let canvas_buffer = client.shm_pool_object(object_id)?.mmap.clone();
            client.register_object(Object::new::<WlBuffer>(
                id,
                ObjectInner::Buffer(Rc::new(RefCell::new(BufferObject {
                    canvas: Canvas::new(
                        Box::new(canvas_buffer),
                        width as usize,
                        height as usize,
                        format,
                    ),
                }))),
            ));
        }
        req => {
            warn!("unhandled request: {:?}", req);
        }
    }

    Ok(())
}
