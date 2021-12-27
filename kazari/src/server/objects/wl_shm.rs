use crate::server::client::Client;

use crate::server::objects::wl_shm_pool::ShmPoolObject;

use crate::server::objects::{Object, ObjectInner};
use crate::server::ServerError;

use crate::utils::mmap::{MMap, MMapAllocator};
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

pub struct ShmObject {}

pub fn handle_request(
    client: &mut Client,
    _object_id: ObjectId,
    req: wl_shm::Request,
    mmap_allocator: &'static dyn MMapAllocator,
) -> Result<(), ServerError> {
    match req {
        wl_shm::Request::CreatePool { id, fd, size } => {
            let mmap = MMap::from_handle(mmap_allocator, fd, size as usize)
                .map_err(ServerError::MMapError)?;
            client.register_object(Object::new::<WlShmPool>(
                id,
                ObjectInner::ShmPool(Rc::new(RefCell::new(ShmPoolObject {
                    mmap: Rc::new(mmap),
                }))),
            ));
        }
    }

    Ok(())
}
