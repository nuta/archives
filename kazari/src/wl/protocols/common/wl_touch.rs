//! The wl_touch interface represents a touchscreen associated with a seat. touch
//! interactions can consist of one or more contacts. for each contact, a series of
//! events is generated, starting with a down event, followed by zero or more motion
//! events, and ending with an up event. events relating to the same contact point
//! can be identified by the id of the sequence.

//
//
//              GENERATED BY OUR WAYLAND-SCANNER. DO NOT EDIT!
//
//

#![allow(unused)]
#![allow(clippy::from_over_into)]
#![allow(clippy::match_single_binding)]

use crate::wl::protocols::common::{EventSet, RequestSet};
use crate::wl::{
    Array, Connection, DeserializeError, Handle, Interface, Message, NewId, ObjectId, Opcode,
    Payload, PayloadType, RawMessage, SendError,
};
use alloc::rc::Rc;
use alloc::string::String;
use core::cell::RefCell;
use smallvec::smallvec;

use crate::wl::protocols::common::wl_buffer::WlBuffer;
use crate::wl::protocols::common::wl_callback::WlCallback;
use crate::wl::protocols::common::wl_compositor::WlCompositor;
use crate::wl::protocols::common::wl_data_device::WlDataDevice;
use crate::wl::protocols::common::wl_data_device_manager::WlDataDeviceManager;
use crate::wl::protocols::common::wl_data_offer::WlDataOffer;
use crate::wl::protocols::common::wl_data_source::WlDataSource;
use crate::wl::protocols::common::wl_display::WlDisplay;
use crate::wl::protocols::common::wl_keyboard::WlKeyboard;
use crate::wl::protocols::common::wl_output::WlOutput;
use crate::wl::protocols::common::wl_pointer::WlPointer;
use crate::wl::protocols::common::wl_region::WlRegion;
use crate::wl::protocols::common::wl_registry::WlRegistry;
use crate::wl::protocols::common::wl_seat::WlSeat;
use crate::wl::protocols::common::wl_shell::WlShell;
use crate::wl::protocols::common::wl_shell_surface::WlShellSurface;
use crate::wl::protocols::common::wl_shm::WlShm;
use crate::wl::protocols::common::wl_shm_pool::WlShmPool;
use crate::wl::protocols::common::wl_subcompositor::WlSubcompositor;
use crate::wl::protocols::common::wl_subsurface::WlSubsurface;
use crate::wl::protocols::common::wl_surface::WlSurface;

use crate::wl::protocols::common::xdg_popup::XdgPopup;
use crate::wl::protocols::common::xdg_positioner::XdgPositioner;
use crate::wl::protocols::common::xdg_surface::XdgSurface;
use crate::wl::protocols::common::xdg_toplevel::XdgToplevel;
use crate::wl::protocols::common::xdg_wm_base::XdgWmBase;

macro_rules! from_optional_object_payload {
    ($ty:ident, $con:expr, $v:expr) => {
        match ($v).clone() {
            Payload::ObjectId(id) if id.is_null() => None,
            Payload::ObjectId(id) => Some($ty::new($con, id)),
            _ => return Err(DeserializeError::UnexpectedType), // Abort deserializing.
        }
    };
}

macro_rules! from_object_payload {
    ($ty:ident, $con:expr, $v:expr) => {
        match ($v).clone() {
            Payload::ObjectId(id) if id.is_null() => return Err(DeserializeError::ObjectIsNull),
            Payload::ObjectId(id) => $ty::new($con, id),
            _ => return Err(DeserializeError::UnexpectedType),
        }
    };
}

macro_rules! from_payload {
    ($ty:ident, $v:expr) => {
        match ($v).clone() {
            Payload::$ty(value) => value.into(),
            _ => return Err(DeserializeError::UnexpectedType),
        }
    };
}

#[derive(Debug)]
pub enum Request {
    /// Release the touch object.
    Release {},
}

impl Message for Request {
    fn into_raw(self, sender: ObjectId) -> RawMessage {
        match self {
            Request::Release {} => RawMessage {
                sender,
                opcode: Opcode(1),
                args: smallvec![],
            },
        }
    }

    fn from_raw(
        con: Rc<RefCell<dyn Connection>>,
        m: &RawMessage,
    ) -> Result<Request, DeserializeError> {
        match m.opcode {
            Opcode(1) => Ok(Request::Release {}),

            _ => Err(DeserializeError::UnknownOpcode),
        }
    }

    fn into_received_event(self, con: Rc<RefCell<dyn Connection>>, id: ObjectId) -> EventSet {
        panic!("not a event!");
    }

    fn into_received_request(self) -> RequestSet {
        RequestSet::WlTouch(self)
    }
}

#[derive(Debug)]
pub enum Event {
    /// A new touch point has appeared on the surface. this touch point is assigned a
    /// unique id. future events from this touch point reference this id. the id ceases
    /// to be valid after a touch up event and may be reused in the future.
    Down {
        /// Serial number of the touch down event.
        serial: u32,
        /// Timestamp with millisecond granularity.
        time: u32,
        /// Surface touched.
        surface: WlSurface,
        /// The unique id of this touch point.
        id: i32,
        /// Surface-local x coordinate.
        x: f32,
        /// Surface-local y coordinate.
        y: f32,
    },
    /// The touch point has disappeared. no further events will be sent for this touch
    /// point and the touch point's id is released and may be reused in a future touch
    /// down event.
    Up {
        /// Serial number of the touch up event.
        serial: u32,
        /// Timestamp with millisecond granularity.
        time: u32,
        /// The unique id of this touch point.
        id: i32,
    },
    /// A touch point has changed coordinates.
    Motion {
        /// Timestamp with millisecond granularity.
        time: u32,
        /// The unique id of this touch point.
        id: i32,
        /// Surface-local x coordinate.
        x: f32,
        /// Surface-local y coordinate.
        y: f32,
    },
    /// Indicates the end of a set of events that logically belong together. a client is
    /// expected to accumulate the data in all events within the frame before
    /// proceeding. a wl_touch.frame terminates at least one event but otherwise no
    /// guarantee is provided about the set of events within a frame. a client must
    /// assume that any state not updated in a frame is unchanged from the previously
    /// known state.
    Frame {},
    /// Sent if the compositor decides the touch stream is a global gesture. no further
    /// events are sent to the clients from that particular gesture. touch cancellation
    /// applies to all touch points currently active on this client's surface. the
    /// client is responsible for finalizing the touch points, future touch points on
    /// this surface may reuse the touch point id.
    Cancel {},
    /// Sent when a touchpoint has changed its shape. this event does not occur on its
    /// own. it is sent before a wl_touch.frame event and carries the new shape
    /// information for any previously reported, or new touch points of that frame.
    /// other events describing the touch point such as wl_touch.down, wl_touch.motion
    /// or wl_touch.orientation may be sent within the same wl_touch.frame. a client
    /// should treat these events as a single logical touch point update. the order of
    /// wl_touch.shape, wl_touch.orientation and wl_touch.motion is not guaranteed. a
    /// wl_touch.down event is guaranteed to occur before the first wl_touch.shape event
    /// for this touch id but both events may occur within the same wl_touch.frame. a
    /// touchpoint shape is approximated by an ellipse through the major and minor axis
    /// length. the major axis length describes the longer diameter of the ellipse,
    /// while the minor axis length describes the shorter diameter. major and minor are
    /// orthogonal and both are specified in surface-local coordinates. the center of
    /// the ellipse is always at the touchpoint location as reported by wl_touch.down or
    /// wl_touch.move. this event is only sent by the compositor if the touch device
    /// supports shape reports. the client has to make reasonable assumptions about the
    /// shape if it did not receive this event.
    Shape {
        /// The unique id of this touch point.
        id: i32,
        /// Length of the major axis in surface-local coordinates.
        major: f32,
        /// Length of the minor axis in surface-local coordinates.
        minor: f32,
    },
    /// Sent when a touchpoint has changed its orientation. this event does not occur on
    /// its own. it is sent before a wl_touch.frame event and carries the new shape
    /// information for any previously reported, or new touch points of that frame.
    /// other events describing the touch point such as wl_touch.down, wl_touch.motion
    /// or wl_touch.shape may be sent within the same wl_touch.frame. a client should
    /// treat these events as a single logical touch point update. the order of
    /// wl_touch.shape, wl_touch.orientation and wl_touch.motion is not guaranteed. a
    /// wl_touch.down event is guaranteed to occur before the first wl_touch.orientation
    /// event for this touch id but both events may occur within the same
    /// wl_touch.frame. the orientation describes the clockwise angle of a touchpoint's
    /// major axis to the positive surface y-axis and is normalized to the -180 to +180
    /// degree range. the granularity of orientation depends on the touch device, some
    /// devices only support binary rotation values between 0 and 90 degrees. this event
    /// is only sent by the compositor if the touch device supports orientation reports.
    Orientation {
        /// The unique id of this touch point.
        id: i32,
        /// Angle between major axis and positive surface y-axis in degrees.
        orientation: f32,
    },
}

impl Message for Event {
    fn into_raw(self, sender: ObjectId) -> RawMessage {
        match self {
            Event::Down {
                serial,
                time,
                surface,
                id,
                x,
                y,
            } => RawMessage {
                sender,
                opcode: Opcode(2),
                args: smallvec![
                    serial.into(),
                    time.into(),
                    surface.into(),
                    id.into(),
                    x.into(),
                    y.into()
                ],
            },
            Event::Up { serial, time, id } => RawMessage {
                sender,
                opcode: Opcode(3),
                args: smallvec![serial.into(), time.into(), id.into()],
            },
            Event::Motion { time, id, x, y } => RawMessage {
                sender,
                opcode: Opcode(4),
                args: smallvec![time.into(), id.into(), x.into(), y.into()],
            },
            Event::Frame {} => RawMessage {
                sender,
                opcode: Opcode(5),
                args: smallvec![],
            },
            Event::Cancel {} => RawMessage {
                sender,
                opcode: Opcode(6),
                args: smallvec![],
            },
            Event::Shape { id, major, minor } => RawMessage {
                sender,
                opcode: Opcode(7),
                args: smallvec![id.into(), major.into(), minor.into()],
            },
            Event::Orientation { id, orientation } => RawMessage {
                sender,
                opcode: Opcode(8),
                args: smallvec![id.into(), orientation.into()],
            },
        }
    }

    fn from_raw(
        con: Rc<RefCell<dyn Connection>>,
        m: &RawMessage,
    ) -> Result<Event, DeserializeError> {
        match m.opcode {
            Opcode(2) => Ok(Event::Down {
                serial: from_payload!(UInt, m.args[0]),

                time: from_payload!(UInt, m.args[1]),

                surface: from_object_payload!(WlSurface, con.clone(), m.args[2]),

                id: from_payload!(Int, m.args[3]),

                x: from_payload!(Fixed, m.args[4]),

                y: from_payload!(Fixed, m.args[5]),
            }),
            Opcode(3) => Ok(Event::Up {
                serial: from_payload!(UInt, m.args[0]),

                time: from_payload!(UInt, m.args[1]),

                id: from_payload!(Int, m.args[2]),
            }),
            Opcode(4) => Ok(Event::Motion {
                time: from_payload!(UInt, m.args[0]),

                id: from_payload!(Int, m.args[1]),

                x: from_payload!(Fixed, m.args[2]),

                y: from_payload!(Fixed, m.args[3]),
            }),
            Opcode(5) => Ok(Event::Frame {}),
            Opcode(6) => Ok(Event::Cancel {}),
            Opcode(7) => Ok(Event::Shape {
                id: from_payload!(Int, m.args[0]),

                major: from_payload!(Fixed, m.args[1]),

                minor: from_payload!(Fixed, m.args[2]),
            }),
            Opcode(8) => Ok(Event::Orientation {
                id: from_payload!(Int, m.args[0]),

                orientation: from_payload!(Fixed, m.args[1]),
            }),

            _ => Err(DeserializeError::UnknownOpcode),
        }
    }

    fn into_received_event(self, con: Rc<RefCell<dyn Connection>>, id: ObjectId) -> EventSet {
        EventSet::WlTouch(WlTouch::new(con, id), self)
    }

    fn into_received_request(self) -> RequestSet {
        panic!("not a request!");
    }
}

/// Touchscreen input device.
#[derive(Clone)]
pub struct WlTouch {
    con: Rc<RefCell<dyn Connection>>,
    object_id: ObjectId,
}

impl PartialEq for WlTouch {
    fn eq(&self, other: &Self) -> bool {
        self.id() == other.id()
    }
}

impl core::fmt::Debug for WlTouch {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        write!(f, "WlTouch@{}", self.object_id.0)
    }
}

impl Into<Payload> for WlTouch {
    fn into(self) -> Payload {
        Payload::ObjectId(self.id())
    }
}

impl Interface for WlTouch {
    type Event = Event;
    type Request = Request;
    const NAME: &'static str = "wl_touch";
    const VERSION: u32 = 7;
    const PAYLOAD_TYPES: &'static [&'static [PayloadType]] = &[
        &[],
        &[
            PayloadType::UInt,
            PayloadType::UInt,
            PayloadType::ObjectId,
            PayloadType::Int,
            PayloadType::Fixed,
            PayloadType::Fixed,
        ],
        &[PayloadType::UInt, PayloadType::UInt, PayloadType::Int],
        &[
            PayloadType::UInt,
            PayloadType::Int,
            PayloadType::Fixed,
            PayloadType::Fixed,
        ],
        &[],
        &[],
        &[PayloadType::Int, PayloadType::Fixed, PayloadType::Fixed],
        &[PayloadType::Int, PayloadType::Fixed],
    ];

    fn new(con: Rc<RefCell<dyn Connection>>, object_id: ObjectId) -> WlTouch {
        WlTouch { con, object_id }
    }

    fn connection(&self) -> &Rc<RefCell<dyn Connection>> {
        &self.con
    }

    fn id(&self) -> ObjectId {
        self.object_id
    }

    fn as_new_id(&self) -> NewId {
        NewId(self.object_id.0)
    }
}