//! The xdg_positioner provides a collection of rules for the placement of a child
//! surface relative to a parent surface. rules can be defined to ensure the child
//! surface remains within the visible area's borders, and to specify how the child
//! surface changes its position, such as sliding along an axis, or flipping around
//! a rectangle. these positioner-created rules are constrained by the requirement
//! that a child surface must intersect with or be at least partially adjacent to
//! its parent surface. see the various requests for details about possible rules.
//! at the time of the request, the compositor makes a copy of the rules specified
//! by the xdg_positioner. thus, after the request is complete the xdg_positioner
//! object can be destroyed or reused; further changes to the object will have no
//! effect on previous usages. for an xdg_positioner object to be considered
//! complete, it must have a non-zero size set by set_size, and a non-zero anchor
//! rectangle set by set_anchor_rect. passing an incomplete xdg_positioner object
//! when positioning a surface raises an error.

//
//
//              GENERATED BY OUR WAYLAND-SCANNER. DO NOT EDIT!
//
//

#![allow(unused)]
#![allow(clippy::from_over_into)]
#![allow(clippy::match_single_binding)]

use crate::wl::{
    Array, Connection, Handle, Interface, Message, NewId, ObjectId, Opcode, Payload, PayloadType,
    RawMessage, SendError,
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
use crate::wl::protocols::common::wl_touch::WlTouch;
use crate::wl::protocols::common::xdg_popup::XdgPopup;

use crate::wl::protocols::common::xdg_positioner::*;
use crate::wl::protocols::common::xdg_surface::XdgSurface;
use crate::wl::protocols::common::xdg_toplevel::XdgToplevel;
use crate::wl::protocols::common::xdg_wm_base::XdgWmBase;

pub trait XdgPositionerExt {}

impl XdgPositionerExt for XdgPositioner {}
