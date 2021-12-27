use crate::server::client::Client;

use crate::server::objects::wl_callback::CallbackObject;
use crate::server::objects::{wl_registry::RegistryObject, GlobalObjectId, Object, ObjectInner};
use crate::server::ServerError;
use crate::utils::interface_wrapper::InterfaceWrapper;
use crate::wl::{
    protocols::common::{
        wl_buffer::{self, WlBuffer},
        wl_callback::{self, WlCallback},
        wl_compositor::{self, WlCompositor},
        wl_display::{self, WlDisplay},
        wl_registry::{self, WlRegistry},
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

pub struct DisplayObject {}

pub fn handle_request(
    client: &mut Client,
    _object_id: ObjectId,
    req: wl_display::Request,
    global_objects: &HashMap<GlobalObjectId, Box<dyn InterfaceWrapper>>,
) -> Result<(), ServerError> {
    match req {
        wl_display::Request::Sync { callback } => {
            client.register_object(Object::new::<WlCallback>(
                callback,
                ObjectInner::Callback(Rc::new(RefCell::new(CallbackObject {}))),
            ));

            let callback_data = client.alloc_serial();
            client.send_event(callback.as_id(), wl_callback::Event::Done { callback_data })?;
        }
        wl_display::Request::GetRegistry { registry } => {
            // Notifies a client all registered global objects.
            for (name, interface) in global_objects {
                client.send_event(
                    registry.as_id(),
                    wl_registry::Event::Global {
                        name: name.0,
                        interface: interface.name().to_string(),
                        version: interface.version(),
                    },
                )?;
            }

            client.register_object(Object::new::<WlRegistry>(
                registry,
                ObjectInner::Registry(Rc::new(RefCell::new(RegistryObject {}))),
            ));
        }
        req => {
            warn!("unhandled request: {:?}", req);
        }
    }

    Ok(())
}
