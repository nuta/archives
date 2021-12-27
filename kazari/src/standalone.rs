use crate::canvas::Canvas;
use crate::client::Client;
use crate::client::{ClientError, Window};
use crate::server::{ClientId, Input, Server};
use crate::wl::protocols::common::wl_callback::{self, WlCallback};
use crate::wl::protocols::common::EventSet;
use crate::wl::{Connection, Handle, RawMessage, SendError};
use crate::{
    Color, FillStyle, FontFamily, FontSize, FontStyle, PixelFormat, Position, RectArea, RectSize,
};
use alloc::collections::VecDeque;
use alloc::rc::Rc;
use alloc::vec::Vec;
use core::cell::RefCell;
use core::cell::RefMut;
pub struct MessageQueue(RefCell<VecDeque<(Vec<u8>, Vec<Handle>)>>);
use crate::canvas::CanvasBuffer;
use crate::mmap::*;
use alloc::boxed::Box;
use once_cell::sync::Lazy;
use std::collections::HashMap;
use std::sync::{
    atomic::{AtomicI32, Ordering},
    Mutex,
};

struct CanvasBufferImpl(Vec<u8>);
impl CanvasBuffer for CanvasBufferImpl {
    fn as_ref(&self) -> &[u8] {
        &self.0
    }

    fn as_mut(&mut self) -> &mut [u8] {
        &mut self.0
    }
}
// FIXME:
static MMAPS: Lazy<Mutex<HashMap<Handle, Vec<u8>>>> = Lazy::new(|| Mutex::new(HashMap::new()));

struct MMapAllocatorImpl {
    next_handle: AtomicI32,
}

impl MMapAllocatorImpl {
    pub const fn new() -> MMapAllocatorImpl {
        MMapAllocatorImpl {
            next_handle: AtomicI32::new(1),
        }
    }
}

impl MMapAllocator for MMapAllocatorImpl {
    fn alloc(&self, len: usize) -> Result<(Handle, *mut u8), NoMemoryError> {
        let handle = Handle(self.next_handle.fetch_add(1, Ordering::SeqCst));
        let mut buf = Vec::with_capacity(len);
        let ptr = buf.as_mut_ptr();
        MMAPS.lock().unwrap().insert(handle, buf);

        trace!("mmap.alloc: handle={:?}", handle);
        Ok((handle, ptr))
    }

    fn map(&self, handle: Handle, len: usize) -> Result<(Handle, *mut u8), MMapError> {
        trace!("mmap.map: handle={:?}", handle);
        let mut mmaps = MMAPS.lock().unwrap();
        let buf = mmaps.get_mut(&handle).unwrap();
        let ptr = buf.as_mut_ptr();
        assert!(len <= buf.capacity());
        Ok((handle, ptr))
    }

    fn free(&self, handle: Handle) {
        trace!("mmap.free: handle={:?}", handle);
    }
}

static MMAP_ALLOCATOR: MMapAllocatorImpl = MMapAllocatorImpl::new();

impl MessageQueue {
    pub fn new() -> MessageQueue {
        MessageQueue(RefCell::new(VecDeque::new()))
    }

    pub fn is_empty(&self) -> bool {
        self.0.borrow().is_empty()
    }

    pub fn receive(&mut self) -> Option<(Vec<u8>, Vec<Handle>)> {
        self.0.borrow_mut().pop_front()
    }
}

impl Connection for MessageQueue {
    fn send(&self, m: RawMessage) -> Result<(), SendError> {
        info!("sending {:?}", m);
        let mut buf = vec![0; 256];
        let mut handles = Vec::new();
        m.serialize(&mut buf, &mut handles)
            .expect("serialize error");
        self.0.borrow_mut().push_back((buf, handles));
        Ok(())
    }
}

pub struct Standalone {
    server: Rc<RefCell<Server>>,
    client: Rc<RefCell<Client>>,
    client_id: ClientId,
    client_queue: Rc<RefCell<MessageQueue>>,
    server_queue: Rc<RefCell<MessageQueue>>,
}

impl Standalone {
    pub fn new(width: usize, height: usize) -> Standalone {
        let buf: Vec<u8> = std::vec![0; height * width * 4];

        // Server.
        let client_queue = Rc::new(RefCell::new(MessageQueue::new()));
        let server_queue = Rc::new(RefCell::new(MessageQueue::new()));
        let client_id = ClientId::new(1);
        let server = Rc::new(RefCell::new(Server::new(
            &MMAP_ALLOCATOR,
            Canvas::new(
                Box::new(CanvasBufferImpl(buf)),
                width,
                height,
                PixelFormat::RGBA8888,
            ),
        )));
        server
            .borrow_mut()
            .add_client(client_id, client_queue.clone());

        // Client.
        let client = Rc::new(RefCell::new(
            Client::new(server_queue.clone(), &MMAP_ALLOCATOR).unwrap(),
        ));

        // Notify/bind global objects. This should not be an inifite loop.
        let roundtrip_cb = client.borrow_mut().roundtrip().unwrap();
        while !client_queue.borrow().is_empty() || !server_queue.borrow().is_empty() {
            if let Some((message, handles)) = client_queue.borrow_mut().receive() {
                let m = client
                    .borrow_mut()
                    .process_message(&message, &handles)
                    .expect("client: process_message error");
                if let Some(EventSet::WlCallback(cb, wl_callback::Event::Done { .. })) = m {
                    if cb == roundtrip_cb {
                        break;
                    }
                }
            }

            if let Some((message, handles)) = server_queue.borrow_mut().receive() {
                server
                    .borrow_mut()
                    .process_message(client_id, &message, &handles)
                    .expect("server: process_message error");
            }
        }

        Standalone {
            server,
            client,
            client_id,
            client_queue,
            server_queue,
        }
    }

    pub fn create_window(&mut self, width: usize, height: usize) -> Result<Window, ClientError> {
        self.client
            .borrow_mut()
            .create_window(RectSize::new(width, height), PixelFormat::RGBA8888)
    }

    pub fn process_input<F>(&mut self, input: Input, callback: F)
    where
        F: FnMut(EventSet),
    {
        self.server.borrow_mut().process_input(input);
        self.do_ping_pong(callback);
    }

    pub fn render<F>(&mut self, callback: F)
    where
        F: Fn(&Canvas),
    {
        self.server.borrow_mut().render();
        callback(self.server.borrow().screen());
    }

    fn do_ping_pong<F>(&mut self, mut callback: F)
    where
        F: FnMut(EventSet),
    {
        while !self.client_queue.borrow().is_empty() || !self.server_queue.borrow().is_empty() {
            if let Some((message, handles)) = self.client_queue.borrow_mut().receive() {
                match self.client.borrow_mut().process_message(&message, &handles) {
                    Ok(Some(ev)) => {
                        callback(ev);
                    }
                    Ok(ev) => {
                        trace!("client: {:?}", ev);
                    }
                    Err(err) => {
                        error!("client: {:?}", err);
                    }
                }
            }

            if let Some((message, handles)) = self.server_queue.borrow_mut().receive() {
                self.server
                    .borrow_mut()
                    .process_message(self.client_id, &message, &handles)
                    .expect("server: process_message error");
            }
        }
    }
}
