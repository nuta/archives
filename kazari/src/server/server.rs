use crate::canvas::Canvas;
use crate::icons::{XpmImage, ICONS};
use crate::server::client::{Client, ClientId};
use crate::server::keyboard::{KeyCode, KeyState};
use crate::server::layer::{Layer, LayerContent};
use crate::server::objects::{self, GlobalObjectId};
use crate::server::pointer::{PointerButton, PointerButtonState, PointerMove};
use crate::utils::interface_wrapper::{construct_interface_wrapper, InterfaceWrapper};
use crate::utils::mmap::{MMapAllocator, MMapError};
use crate::wl::protocols::common::wl_keyboard;
use crate::wl::protocols::server::wl_keyboard::WlKeyboardExt;
use crate::wl::Array;
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
use crate::{Color, FillStyle, Position, RectArea, RectSize};
use alloc::boxed::Box;
use alloc::rc::Rc;
use alloc::vec::Vec;
use core::cell::{Ref, RefCell, RefMut};
use core::cmp::min;
use hashbrown::HashMap;

#[derive(Debug, PartialEq)]
pub enum ServerError {
    ClientNotFound,
    ObjectMismatch,
    UnknownGlobal,
    MMapError(MMapError),
    SendError(SendError),
    MessageDeserializeError(DeserializeError),
    MalformedMessageError,
    UnknownObjectId(ObjectId),
    UnknownOpcode(&'static str, ObjectId, Opcode),
}

#[derive(Debug, Clone)]
pub enum Input {
    /// Pressing and releasing keys on a keyboard.
    Keyboard { state: KeyState, code: KeyCode },
    /// Button clicks.
    PointerButton {
        state: PointerButtonState,
        button: PointerButton,
    },
    /// Moving a mouse.
    PointerMove(PointerMove),
}

pub struct Server {
    global_objects: HashMap<GlobalObjectId, Box<dyn InterfaceWrapper>>,
    clients: HashMap<ClientId, Client>,
    mmap_allocator: &'static dyn MMapAllocator,
    screen: Canvas,
    damaged_areas: Vec<RectArea>,
    cursor_layer: Layer,
    background_layer: Layer,
    layers: Vec<Layer>,
    active_layer_index: Option<usize>,
    layer_in_drag: Option<(usize, Position)>,
    pointer_left_button: PointerButtonState,
    pointer_right_button: PointerButtonState,
}

impl Server {
    pub fn new(mmap_allocator: &'static dyn MMapAllocator, screen: Canvas) -> Server {
        let mut global_objects = HashMap::new();
        global_objects.insert(
            GlobalObjectId(1),
            construct_interface_wrapper::<WlCompositor>(),
        );
        global_objects.insert(GlobalObjectId(2), construct_interface_wrapper::<WlShm>());
        global_objects.insert(GlobalObjectId(3), construct_interface_wrapper::<WlSeat>());
        global_objects.insert(
            GlobalObjectId(4),
            construct_interface_wrapper::<XdgWmBase>(),
        );

        let damaged_areas = vec![RectArea::new(Position::new(0, 0), screen.size())];
        let cursor_layer = Layer::new(
            Position::new(10, 10),
            RectSize::new(ICONS.normal_cursor.width, ICONS.normal_cursor.height),
            LayerContent::XpmImage {
                image: &ICONS.normal_cursor,
            },
            false,
        );

        let background_layer = Layer::new(
            Position::new(0, 0),
            screen.size(),
            LayerContent::SolidColor {
                color: Color::Rgba8888 {
                    r: 00,
                    g: 128,
                    b: 127,
                    a: 255,
                },
            },
            false,
        );
        Server {
            mmap_allocator,
            global_objects,
            clients: HashMap::new(),
            screen,
            damaged_areas,
            layers: Vec::new(),
            cursor_layer,
            background_layer,
            active_layer_index: None,
            layer_in_drag: None,
            pointer_left_button: PointerButtonState::Released,
            pointer_right_button: PointerButtonState::Released,
        }
    }

    /// The canvas which holds the screen content.
    pub fn screen(&self) -> &Canvas {
        &self.screen
    }

    /// The current screen size.
    pub fn screen_size(&self) -> RectSize {
        self.screen.size()
    }

    /// Registers a new client.
    pub fn add_client(&mut self, id: ClientId, con: Rc<RefCell<dyn Connection>>) {
        self.clients.insert(id, Client::new(id, con));
    }

    /// Parses and processes a (request) message.
    pub fn process_message(
        &mut self,
        client_id: ClientId,
        message: &[u8],
        handles: &[Handle],
    ) -> Result<(), ServerError> {
        let (object_id, opcode) = RawMessage::deserialize_header(message)
            .map_err(ServerError::MessageDeserializeError)?;

        // Look for the object control block.
        let client = self
            .clients
            .get_mut(&client_id)
            .ok_or(ServerError::ClientNotFound)?;

        let received_request = client
            .object_interface(object_id)?
            .as_received_request(
                client.connection().clone(),
                opcode,
                object_id,
                message,
                handles,
            )
            .map_err(ServerError::MessageDeserializeError)?;

        trace!("S: received: {:?}", received_request);
        match received_request {
            RequestSet::WlDisplay(req) => {
                objects::wl_display::handle_request(client, object_id, req, &self.global_objects)?;
            }
            RequestSet::WlCompositor(req) => {
                objects::wl_compositor::handle_request(client, object_id, req)?;
            }
            RequestSet::WlRegistry(req) => {
                objects::wl_registry::handle_request(client, object_id, req, &self.global_objects)?;
            }
            RequestSet::WlSeat(req) => {
                objects::wl_seat::handle_request(client, object_id, req)?;
            }
            RequestSet::WlKeyboard(req) => {
                objects::wl_keyboard::handle_request(client, object_id, req)?;
            }
            RequestSet::WlShm(req) => {
                objects::wl_shm::handle_request(client, object_id, req, self.mmap_allocator)?;
            }
            RequestSet::WlShmPool(req) => {
                objects::wl_shm_pool::handle_request(client, object_id, req)?;
            }
            RequestSet::WlSurface(req) => {
                let changes = objects::wl_surface::handle_request(client, object_id, req)?;
                if let Some(area) = changes.damaged_area {
                    for layer in &self.layers {
                        let resolved_area = match layer.content() {
                            LayerContent::Window { surface, .. }
                                if object_id == surface.borrow().object_id =>
                            {
                                Some(RectArea::new(layer.position() + area.pos, area.size))
                            }
                            _ => None,
                        };

                        if let Some(resolved_area) = resolved_area {
                            self.mark_as_damaged(resolved_area);
                            break;
                        }
                    }
                }
            }
            RequestSet::XdgWmBase(req) => {
                objects::xdg_wm_base::handle_request(client, object_id, req)?;
            }
            RequestSet::XdgSurface(req) => {
                objects::xdg_surface::handle_request(client, object_id, req, &mut self.layers)?;
            }
            RequestSet::XdgToplevel(req) => {
                objects::xdg_toplevel::handle_request(client, object_id, req)?;
            }
            req => {
                warn!("unhandled request: {:?}", req);
            }
        };

        Ok(())
    }

    /// Processes a mouse/keyboard input.
    pub fn process_input(&mut self, input: Input) {
        match input {
            Input::Keyboard { state, code } => {
                if let Some(layer) = self.active_layer() {
                    if let LayerContent::Window { surface, .. } = layer.content() {
                        let state = match state {
                            KeyState::Pressed => wl_keyboard::KeyState::Pressed,
                            KeyState::Released => wl_keyboard::KeyState::Released,
                        };

                        let client_id = surface.borrow().client_id;
                        let client = self.clients.get_mut(&client_id).unwrap();
                        let serial = client.alloc_serial();
                        if let Some(keyboard) = client.keyboard_object() {
                            keyboard.key(serial, 0, code as u32, state);
                        }
                    }
                }
            }
            Input::PointerButton {
                state,
                button: PointerButton::Left,
            } => {
                self.pointer_left_button = state;
                self.layer_in_drag = match (state, self.layer_at_pointer()) {
                    (PointerButtonState::Pressed, Some(index)) => {
                        let layer = &self.layers[index];
                        let layer_area = layer.area();
                        let relative_pos = self.cursor_layer.position() - layer.position();

                        if let LayerContent::Window { surface, .. } = layer.content() {
                            let client_id = surface.borrow().client_id;
                            let client = self.clients.get_mut(&client_id).unwrap();
                            let serial = client.alloc_serial();
                            if let Some(keyboard) = client.keyboard_object() {
                                keyboard.enter(
                                    serial,
                                    WlSurface::new(
                                        client.connection().clone(),
                                        surface.borrow().object_id,
                                    ),
                                    Array::from_bytes(&[]),
                                );
                            }

                            self.active_layer_index = Some(index);
                            self.mark_as_damaged(layer_area);
                        }
                        Some((index, relative_pos))
                    }
                    _ => None,
                };
            }
            Input::PointerButton {
                state,
                button: PointerButton::Right,
            } => {
                self.pointer_right_button = state;
            }
            Input::PointerMove(move_) => {
                let unclamped_new_cursor_pos = match move_ {
                    PointerMove::Relative { x_offset, y_offset } => {
                        unimplemented!()
                    }
                    PointerMove::Absolute { new_pos } => new_pos,
                };

                let new_cursor_pos = unclamped_new_cursor_pos.clamp(
                    Position::zero(),
                    Position::new(self.screen_size().width - 1, self.screen_size().height - 1),
                );

                let old_area = self.cursor_layer.area();
                self.cursor_layer.move_to(new_cursor_pos);
                self.mark_as_damaged(old_area);
                self.mark_as_damaged(self.cursor_layer.area());

                if let Some((index, relative_pos)) = self.layer_in_drag.clone() {
                    let new_layer_pos = new_cursor_pos - relative_pos;
                    let old_area = self.layers[index].area();
                    self.mark_as_damaged(old_area);
                    self.layers[index].move_to(new_layer_pos);
                    self.mark_as_damaged(self.layers[index].area());
                }
            }
        }
    }

    /// Marks a part of the screen as damaged.
    pub fn mark_as_damaged(&mut self, area: RectArea) {
        if let Some(damaged_area) =
            area.overlapping(RectArea::new(Position::zero(), self.screen_size()))
        {
            self.damaged_areas.push(damaged_area);
        }
    }

    /// Renders contents into the screen.
    pub fn render(&mut self) {
        // trace!("damaged_areas: {:?}", self.damaged_areas);
        for damaged_area in self.damaged_areas.drain(..) {
            // Draw the wallpaper.
            self.background_layer
                .render(&mut self.screen, damaged_area, false);

            // Draw from the bottom-most layer.
            for (i, layer) in self.layers.iter().enumerate() {
                let active = self.active_layer_index.map(|j| i == j).unwrap_or(false);
                layer.render(&mut self.screen, damaged_area, active);
            }

            // Draw the cursor.
            self.cursor_layer
                .render(&mut self.screen, damaged_area, false);
        }
    }

    fn active_layer(&self) -> Option<&Layer> {
        self.active_layer_index
            .as_ref()
            .map(|index| &self.layers[*index])
    }

    fn layer_at_pointer(&self) -> Option<usize> {
        let cursor_pos = self.cursor_layer.position();
        for (i, layer) in self.layers.iter().rev().enumerate() {
            if layer.draggable() && layer.area().contains_position(cursor_pos) {
                return Some(self.layers.len() - 1 - i);
            }
        }

        None
    }
}
