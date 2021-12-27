pub mod window;

use crate::utils::interface_wrapper::construct_interface_wrapper;
use crate::utils::interface_wrapper::InterfaceWrapper;
use crate::utils::mmap::{MMap, MMapAllocator, NoMemoryError};
use crate::wl::{
    protocols::{
        client::{
            wl_compositor::WlCompositorExt, wl_display::WlDisplayExt, wl_registry::WlRegistryExt,
            wl_shm::WlShmExt, wl_shm_pool::WlShmPoolExt, wl_surface::WlSurfaceExt,
            xdg_surface::XdgSurfaceExt, xdg_wm_base::XdgWmBaseExt,
        },
        common::{
            wl_buffer::WlBuffer, wl_callback::WlCallback, wl_compositor::WlCompositor,
            wl_display::WlDisplay, wl_registry, wl_registry::WlRegistry, wl_seat::WlSeat,
            wl_shm::Format, wl_shm::WlShm, wl_shm_pool::WlShmPool, wl_surface::WlSurface,
            xdg_surface::XdgSurface, xdg_toplevel::XdgToplevel, xdg_wm_base::XdgWmBase, EventSet,
        },
    },
    Connection, DeserializeError, Handle, Interface, NewId, ObjectId, Opcode, RawMessage,
    SendError,
};
use crate::{PixelFormat, RectSize};
use alloc::boxed::Box;
use alloc::rc::Rc;
use core::cell::RefCell;
use hashbrown::HashMap;
pub use window::Window;

#[derive(Debug, PartialEq)]
pub enum ClientError {
    MMapError(NoMemoryError),
    SendError(SendError),
    MessageDeserializeError(DeserializeError),
    MalformedMessageError,
    UnknownObjectId(ObjectId),
    UnknownOpcode(&'static str, ObjectId, Opcode),
}

struct Object {
    /// The interface.
    interface: Box<dyn InterfaceWrapper>,
}

pub struct Client {
    con: Rc<RefCell<dyn Connection>>,
    mmap_allocator: &'static dyn MMapAllocator,
    next_object_id: u32,
    display: WlDisplay,
    global_objects: HashMap<&'static str, ObjectId>,
    objects: HashMap<ObjectId, Object>,
}

impl Client {
    pub fn new(
        con: Rc<RefCell<dyn Connection>>,
        mmap_allocator: &'static dyn MMapAllocator,
    ) -> Result<Client, ClientError> {
        let mut objects = HashMap::new();
        let mut display = WlDisplay::new(con.clone(), ObjectId(1));
        objects.insert(
            ObjectId(1),
            Object {
                interface: construct_interface_wrapper::<WlDisplay>(),
            },
        );
        let mut registry = WlRegistry::new(con.clone(), ObjectId(2));
        objects.insert(
            ObjectId(2),
            Object {
                interface: construct_interface_wrapper::<WlRegistry>(),
            },
        );

        display
            .get_registry(registry.as_new_id())
            .map_err(ClientError::SendError)?;

        Ok(Client {
            con,
            mmap_allocator,
            next_object_id: 3,
            display,
            global_objects: HashMap::new(),
            objects,
        })
    }

    fn allocate_object<I: Interface>(&mut self) -> I {
        let id = ObjectId(self.next_object_id);
        self.next_object_id += 1;
        self.objects.insert(
            id,
            Object {
                interface: construct_interface_wrapper::<I>(),
            },
        );
        I::new(self.con.clone(), id)
    }

    /// Looks for a global object. Returns `None` if the global object is not
    /// notified from the server.
    pub fn lookup_global_object<I: Interface>(&self) -> Option<I> {
        self.global_objects
            .get(I::NAME)
            .map(|id| I::new(self.con.clone(), *id))
    }

    /// Creates an application window.
    pub fn create_window(
        &mut self,
        size: RectSize,
        format: PixelFormat,
    ) -> Result<Window, ClientError> {
        let (shm_format, stride) = match format {
            PixelFormat::RGBA8888 => (Format::Rgba8888, size.width * 4),
        };

        let mut shm: WlShm = self
            .lookup_global_object()
            .expect("shm object is not provided");
        let mut xdg_wm_base: XdgWmBase = self
            .lookup_global_object()
            .expect("xdg_wm_base object is not provided");
        let wl_seat: WlSeat = self
            .lookup_global_object()
            .expect("wl_seat object is not provided");
        let mut wl_compositor: WlCompositor = self
            .lookup_global_object()
            .expect("wl_compositor object is not provided");

        // Create a shm_pool to allocate a frame buffer.
        let mmap_len = stride * size.height;
        let (mmap_handle, mmap_ptr) = self
            .mmap_allocator
            .alloc(mmap_len)
            .map_err(ClientError::MMapError)?;
        let mmap = MMap::new(self.mmap_allocator, mmap_handle, mmap_ptr, mmap_len);

        let mut shm_pool: WlShmPool = self.allocate_object();
        shm.create_pool(shm_pool.as_new_id(), mmap.handle(), mmap.len() as i32)
            .map_err(ClientError::SendError)?;

        // Allocate a buffer for the window contents.
        let buffer: WlBuffer = self.allocate_object();
        shm_pool
            .create_buffer(
                NewId(buffer.id().0),
                0,
                size.width as i32,
                size.height as i32,
                stride as i32,
                shm_format,
            )
            .map_err(ClientError::SendError)?;

        // Attach the buffer to the window.
        let mut surface: WlSurface = self.allocate_object();
        wl_compositor
            .create_surface(surface.as_new_id())
            .map_err(ClientError::SendError)?;
        surface
            .attach(Some(buffer), 0, 0)
            .map_err(ClientError::SendError)?;

        // Create an application window.
        let mut xdg_surface: XdgSurface = self.allocate_object();
        xdg_wm_base
            .get_xdg_surface(xdg_surface.as_new_id(), surface.clone())
            .map_err(ClientError::SendError)?;
        let xdg_toplevel: XdgToplevel = self.allocate_object();
        xdg_surface
            .get_toplevel(xdg_toplevel.as_new_id())
            .map_err(ClientError::SendError)?;

        // FIXME: The server should damage the whole buffer.
        surface
            .damage(0, 0, size.width as i32, size.height as i32)
            .map_err(ClientError::SendError)?;

        Ok(Window::new(
            wl_seat,
            self.allocate_object(),
            self.allocate_object(),
            xdg_surface,
            xdg_toplevel,
            surface,
            Rc::new(mmap),
            size,
            format,
        ))
    }

    /// Parses a (event) message. If the message is successfully handled
    /// internally, it returns `Ok(None)`.
    pub fn process_message(
        &mut self,
        message: &[u8],
        handles: &[Handle],
    ) -> Result<Option<EventSet>, ClientError> {
        let (object_id, opcode) = RawMessage::deserialize_header(message)
            .map_err(ClientError::MessageDeserializeError)?;

        // Look for the object control block.
        let object = match self.objects.get_mut(&object_id) {
            Some(object) => object,
            None => return Err(ClientError::UnknownObjectId(object_id)),
        };

        let ev = object
            .interface
            .as_received_event(self.con.clone(), opcode, object_id, message, handles)
            .map_err(ClientError::MessageDeserializeError)?;

        match ev {
            EventSet::WlRegistry(mut registry, ev) => match ev {
                wl_registry::Event::Global {
                    name,
                    interface,
                    version,
                } => {
                    let (interface_name, new_id) = match interface.as_str() {
                        WlShm::NAME => (WlShm::NAME, self.allocate_object::<WlShm>().as_new_id()),
                        WlCompositor::NAME => (
                            WlCompositor::NAME,
                            self.allocate_object::<WlCompositor>().as_new_id(),
                        ),
                        WlSeat::NAME => {
                            (WlSeat::NAME, self.allocate_object::<WlSeat>().as_new_id())
                        }
                        XdgWmBase::NAME => (
                            XdgWmBase::NAME,
                            self.allocate_object::<XdgWmBase>().as_new_id(),
                        ),
                        _ => return Ok(None),
                    };

                    registry
                        .bind(name, new_id)
                        .map_err(ClientError::SendError)?;
                    self.global_objects.insert(interface_name, new_id.as_id());
                }
                _ => {}
            },
            _ => return Ok(Some(ev)),
        }

        Ok(None)
    }

    /// Requests ana asychronous roundtrip (`wl_display::sync`).
    pub fn roundtrip(&mut self) -> Result<WlCallback, ClientError> {
        let callback: WlCallback = self.allocate_object();
        self.display
            .sync(callback.as_new_id())
            .map_err(ClientError::SendError)?;
        Ok(callback)
    }
}
