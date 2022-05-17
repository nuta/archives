//! This interface defines an xdg_surface role which allows a surface to, among
//! other things, set window-like properties such as maximize, fullscreen, and
//! minimize, set application-specific metadata like title and id, and well as
//! trigger user interactive operations such as interactive resize and move.
//! unmapping an xdg_toplevel means that the surface cannot be shown by the
//! compositor until it is explicitly mapped again. all active operations (e.g.,
//! move, resize) are canceled and all attributes (e.g. title, state, stacking, ...)
//! are discarded for an xdg_toplevel surface when it is unmapped. the xdg_toplevel
//! returns to the state it had right after xdg_surface.get_toplevel. the client can
//! re-map the toplevel by perfoming a commit without any buffer attached, waiting
//! for a configure event and handling it as usual (see xdg_surface description).
//! attaching a null buffer to a toplevel unmaps the surface.

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
use crate::wl::protocols::common::xdg_positioner::XdgPositioner;
use crate::wl::protocols::common::xdg_surface::XdgSurface;

use crate::wl::protocols::common::xdg_toplevel::*;
use crate::wl::protocols::common::xdg_wm_base::XdgWmBase;

pub trait XdgToplevelExt {
    /// This request destroys the role surface and unmaps the surface; see "unmapping"
    /// behavior in interface section for details.
    fn destroy(&self) -> Result<(), SendError>;
    /// Set the "parent" of this surface. this surface should be stacked above the
    /// parent surface and all other ancestor surfaces. parent windows should be set on
    /// dialogs, toolboxes, or other "auxiliary" surfaces, so that the parent is raised
    /// when the dialog is raised. setting a null parent for a child window removes any
    /// parent-child relationship for the child. setting a null parent for a window
    /// which currently has no parent is a no-op. if the parent is unmapped then its
    /// children are managed as though the parent of the now-unmapped parent has become
    /// the parent of this surface. if no parent exists for the now-unmapped parent then
    /// the children are managed as though they have no parent surface.
    fn set_parent(&self, parent: Option<XdgToplevel>) -> Result<(), SendError>;
    /// Set a short title for the surface. this string may be used to identify the
    /// surface in a task bar, window list, or other user interface elements provided by
    /// the compositor. the string must be encoded in utf-8.
    fn set_title(&self, title: String) -> Result<(), SendError>;
    /// Set an application identifier for the surface. the app id identifies the general
    /// class of applications to which the surface belongs. the compositor can use this
    /// to group multiple surfaces together, or to determine how to launch a new
    /// application. for d-bus activatable applications, the app id is used as the d-bus
    /// service name. the compositor shell will try to group application surfaces
    /// together by their app id. as a best practice, it is suggested to select app id's
    /// that match the basename of the application's .desktop file. for example,
    /// "org.freedesktop.fooviewer" where the .desktop file is
    /// "org.freedesktop.fooviewer.desktop". like other properties, a set_app_id request
    /// can be sent after the xdg_toplevel has been mapped to update the property. see
    /// the desktop-entry specification [0] for more details on application identifiers
    /// and how they relate to well-known d-bus names and .desktop files. [0]
    /// http://standards.freedesktop.org/desktop-entry-spec/.
    fn set_app_id(&self, app_id: String) -> Result<(), SendError>;
    /// Clients implementing client-side decorations might want to show a context menu
    /// when right-clicking on the decorations, giving the user a menu that they can use
    /// to maximize or minimize the window. this request asks the compositor to pop up
    /// such a window menu at the given position, relative to the local surface
    /// coordinates of the parent surface. there are no guarantees as to what menu items
    /// the window menu contains. this request must be used in response to some sort of
    /// user action like a button press, key press, or touch down event.
    fn show_window_menu(&self, seat: WlSeat, serial: u32, x: i32, y: i32) -> Result<(), SendError>;
    /// Start an interactive, user-driven move of the surface. this request must be used
    /// in response to some sort of user action like a button press, key press, or touch
    /// down event. the passed serial is used to determine the type of interactive move
    /// (touch, pointer, etc). the server may ignore move requests depending on the
    /// state of the surface (e.g. fullscreen or maximized), or if the passed serial is
    /// no longer valid. if triggered, the surface will lose the focus of the device
    /// (wl_pointer, wl_touch, etc) used for the move. it is up to the compositor to
    /// visually indicate that the move is taking place, such as updating a pointer
    /// cursor, during the move. there is no guarantee that the device focus will return
    /// when the move is completed.
    fn move_(&self, seat: WlSeat, serial: u32) -> Result<(), SendError>;
    /// Start a user-driven, interactive resize of the surface. this request must be
    /// used in response to some sort of user action like a button press, key press, or
    /// touch down event. the passed serial is used to determine the type of interactive
    /// resize (touch, pointer, etc). the server may ignore resize requests depending on
    /// the state of the surface (e.g. fullscreen or maximized). if triggered, the
    /// client will receive configure events with the "resize" state enum value and the
    /// expected sizes. see the "resize" enum value for more details about what is
    /// required. the client must also acknowledge configure events using
    /// "ack_configure". after the resize is completed, the client will receive another
    /// "configure" event without the resize state. if triggered, the surface also will
    /// lose the focus of the device (wl_pointer, wl_touch, etc) used for the resize. it
    /// is up to the compositor to visually indicate that the resize is taking place,
    /// such as updating a pointer cursor, during the resize. there is no guarantee that
    /// the device focus will return when the resize is completed. the edges parameter
    /// specifies how the surface should be resized, and is one of the values of the
    /// resize_edge enum. the compositor may use this information to update the surface
    /// position for example when dragging the top left corner. the compositor may also
    /// use this information to adapt its behavior, e.g. choose an appropriate cursor
    /// image.
    fn resize(&self, seat: WlSeat, serial: u32, edges: ResizeEdge) -> Result<(), SendError>;
    /// Set a maximum size for the window. the client can specify a maximum size so that
    /// the compositor does not try to configure the window beyond this size. the width
    /// and height arguments are in window geometry coordinates. see
    /// xdg_surface.set_window_geometry. values set in this way are double-buffered.
    /// they will get applied on the next commit. the compositor can use this
    /// information to allow or disallow different states like maximize or fullscreen
    /// and draw accurate animations. similarly, a tiling window manager may use this
    /// information to place and resize client windows in a more effective way. the
    /// client should not rely on the compositor to obey the maximum size. the
    /// compositor may decide to ignore the values set by the client and request a
    /// larger size. if never set, or a value of zero in the request, means that the
    /// client has no expected maximum size in the given dimension. as a result, a
    /// client wishing to reset the maximum size to an unspecified state can use zero
    /// for width and height in the request. requesting a maximum size to be smaller
    /// than the minimum size of a surface is illegal and will result in a protocol
    /// error. the width and height must be greater than or equal to zero. using
    /// strictly negative values for width and height will result in a protocol error.
    fn set_max_size(&self, width: i32, height: i32) -> Result<(), SendError>;
    /// Set a minimum size for the window. the client can specify a minimum size so that
    /// the compositor does not try to configure the window below this size. the width
    /// and height arguments are in window geometry coordinates. see
    /// xdg_surface.set_window_geometry. values set in this way are double-buffered.
    /// they will get applied on the next commit. the compositor can use this
    /// information to allow or disallow different states like maximize or fullscreen
    /// and draw accurate animations. similarly, a tiling window manager may use this
    /// information to place and resize client windows in a more effective way. the
    /// client should not rely on the compositor to obey the minimum size. the
    /// compositor may decide to ignore the values set by the client and request a
    /// smaller size. if never set, or a value of zero in the request, means that the
    /// client has no expected minimum size in the given dimension. as a result, a
    /// client wishing to reset the minimum size to an unspecified state can use zero
    /// for width and height in the request. requesting a minimum size to be larger than
    /// the maximum size of a surface is illegal and will result in a protocol error.
    /// the width and height must be greater than or equal to zero. using strictly
    /// negative values for width and height will result in a protocol error.
    fn set_min_size(&self, width: i32, height: i32) -> Result<(), SendError>;
    /// Maximize the surface. after requesting that the surface should be maximized, the
    /// compositor will respond by emitting a configure event. whether this configure
    /// actually sets the window maximized is subject to compositor policies. the client
    /// must then update its content, drawing in the configured state. the client must
    /// also acknowledge the configure when committing the new content (see
    /// ack_configure). it is up to the compositor to decide how and where to maximize
    /// the surface, for example which output and what region of the screen should be
    /// used. if the surface was already maximized, the compositor will still emit a
    /// configure event with the "maximized" state. if the surface is in a fullscreen
    /// state, this request has no direct effect. it may alter the state the surface is
    /// returned to when unmaximized unless overridden by the compositor.
    fn set_maximized(&self) -> Result<(), SendError>;
    /// Unmaximize the surface. after requesting that the surface should be unmaximized,
    /// the compositor will respond by emitting a configure event. whether this actually
    /// un-maximizes the window is subject to compositor policies. if available and
    /// applicable, the compositor will include the window geometry dimensions the
    /// window had prior to being maximized in the configure event. the client must then
    /// update its content, drawing it in the configured state. the client must also
    /// acknowledge the configure when committing the new content (see ack_configure).
    /// it is up to the compositor to position the surface after it was unmaximized;
    /// usually the position the surface had before maximizing, if applicable. if the
    /// surface was already not maximized, the compositor will still emit a configure
    /// event without the "maximized" state. if the surface is in a fullscreen state,
    /// this request has no direct effect. it may alter the state the surface is
    /// returned to when unmaximized unless overridden by the compositor.
    fn unset_maximized(&self) -> Result<(), SendError>;
    /// Make the surface fullscreen. after requesting that the surface should be
    /// fullscreened, the compositor will respond by emitting a configure event. whether
    /// the client is actually put into a fullscreen state is subject to compositor
    /// policies. the client must also acknowledge the configure when committing the new
    /// content (see ack_configure). the output passed by the request indicates the
    /// client's preference as to which display it should be set fullscreen on. if this
    /// value is null, it's up to the compositor to choose which display will be used to
    /// map this surface. if the surface doesn't cover the whole output, the compositor
    /// will position the surface in the center of the output and compensate with with
    /// border fill covering the rest of the output. the content of the border fill is
    /// undefined, but should be assumed to be in some way that attempts to blend into
    /// the surrounding area (e.g. solid black). if the fullscreened surface is not
    /// opaque, the compositor must make sure that other screen content not part of the
    /// same surface tree (made up of subsurfaces, popups or similarly coupled surfaces)
    /// are not visible below the fullscreened surface.
    fn set_fullscreen(&self, output: Option<WlOutput>) -> Result<(), SendError>;
    /// Make the surface no longer fullscreen. after requesting that the surface should
    /// be unfullscreened, the compositor will respond by emitting a configure event.
    /// whether this actually removes the fullscreen state of the client is subject to
    /// compositor policies. making a surface unfullscreen sets states for the surface
    /// based on the following: * the state(s) it may have had before becoming
    /// fullscreen * any state(s) decided by the compositor * any state(s) requested by
    /// the client while the surface was fullscreen the compositor may include the
    /// previous window geometry dimensions in the configure event, if applicable. the
    /// client must also acknowledge the configure when committing the new content (see
    /// ack_configure).
    fn unset_fullscreen(&self) -> Result<(), SendError>;
    /// Request that the compositor minimize your surface. there is no way to know if
    /// the surface is currently minimized, nor is there any way to unset minimization
    /// on this surface. if you are looking to throttle redrawing when minimized, please
    /// instead use the wl_surface.frame event for this, as this will also work with
    /// live previews on windows in alt-tab, expose or similar compositor features.
    fn set_minimized(&self) -> Result<(), SendError>;
}

impl XdgToplevelExt for XdgToplevel {
    /// This request destroys the role surface and unmaps the surface; see "unmapping"
    /// behavior in interface section for details.
    fn destroy(&self) -> Result<(), SendError> {
        self.connection()
            .borrow_mut()
            .send(Request::Destroy {}.into_raw(self.id()))
    }
    /// Set the "parent" of this surface. this surface should be stacked above the
    /// parent surface and all other ancestor surfaces. parent windows should be set on
    /// dialogs, toolboxes, or other "auxiliary" surfaces, so that the parent is raised
    /// when the dialog is raised. setting a null parent for a child window removes any
    /// parent-child relationship for the child. setting a null parent for a window
    /// which currently has no parent is a no-op. if the parent is unmapped then its
    /// children are managed as though the parent of the now-unmapped parent has become
    /// the parent of this surface. if no parent exists for the now-unmapped parent then
    /// the children are managed as though they have no parent surface.
    fn set_parent(&self, parent: Option<XdgToplevel>) -> Result<(), SendError> {
        self.connection()
            .borrow_mut()
            .send(Request::SetParent { parent }.into_raw(self.id()))
    }
    /// Set a short title for the surface. this string may be used to identify the
    /// surface in a task bar, window list, or other user interface elements provided by
    /// the compositor. the string must be encoded in utf-8.
    fn set_title(&self, title: String) -> Result<(), SendError> {
        self.connection()
            .borrow_mut()
            .send(Request::SetTitle { title }.into_raw(self.id()))
    }
    /// Set an application identifier for the surface. the app id identifies the general
    /// class of applications to which the surface belongs. the compositor can use this
    /// to group multiple surfaces together, or to determine how to launch a new
    /// application. for d-bus activatable applications, the app id is used as the d-bus
    /// service name. the compositor shell will try to group application surfaces
    /// together by their app id. as a best practice, it is suggested to select app id's
    /// that match the basename of the application's .desktop file. for example,
    /// "org.freedesktop.fooviewer" where the .desktop file is
    /// "org.freedesktop.fooviewer.desktop". like other properties, a set_app_id request
    /// can be sent after the xdg_toplevel has been mapped to update the property. see
    /// the desktop-entry specification [0] for more details on application identifiers
    /// and how they relate to well-known d-bus names and .desktop files. [0]
    /// http://standards.freedesktop.org/desktop-entry-spec/.
    fn set_app_id(&self, app_id: String) -> Result<(), SendError> {
        self.connection()
            .borrow_mut()
            .send(Request::SetAppId { app_id }.into_raw(self.id()))
    }
    /// Clients implementing client-side decorations might want to show a context menu
    /// when right-clicking on the decorations, giving the user a menu that they can use
    /// to maximize or minimize the window. this request asks the compositor to pop up
    /// such a window menu at the given position, relative to the local surface
    /// coordinates of the parent surface. there are no guarantees as to what menu items
    /// the window menu contains. this request must be used in response to some sort of
    /// user action like a button press, key press, or touch down event.
    fn show_window_menu(&self, seat: WlSeat, serial: u32, x: i32, y: i32) -> Result<(), SendError> {
        self.connection()
            .borrow_mut()
            .send(Request::ShowWindowMenu { seat, serial, x, y }.into_raw(self.id()))
    }
    /// Start an interactive, user-driven move of the surface. this request must be used
    /// in response to some sort of user action like a button press, key press, or touch
    /// down event. the passed serial is used to determine the type of interactive move
    /// (touch, pointer, etc). the server may ignore move requests depending on the
    /// state of the surface (e.g. fullscreen or maximized), or if the passed serial is
    /// no longer valid. if triggered, the surface will lose the focus of the device
    /// (wl_pointer, wl_touch, etc) used for the move. it is up to the compositor to
    /// visually indicate that the move is taking place, such as updating a pointer
    /// cursor, during the move. there is no guarantee that the device focus will return
    /// when the move is completed.
    fn move_(&self, seat: WlSeat, serial: u32) -> Result<(), SendError> {
        self.connection()
            .borrow_mut()
            .send(Request::Move { seat, serial }.into_raw(self.id()))
    }
    /// Start a user-driven, interactive resize of the surface. this request must be
    /// used in response to some sort of user action like a button press, key press, or
    /// touch down event. the passed serial is used to determine the type of interactive
    /// resize (touch, pointer, etc). the server may ignore resize requests depending on
    /// the state of the surface (e.g. fullscreen or maximized). if triggered, the
    /// client will receive configure events with the "resize" state enum value and the
    /// expected sizes. see the "resize" enum value for more details about what is
    /// required. the client must also acknowledge configure events using
    /// "ack_configure". after the resize is completed, the client will receive another
    /// "configure" event without the resize state. if triggered, the surface also will
    /// lose the focus of the device (wl_pointer, wl_touch, etc) used for the resize. it
    /// is up to the compositor to visually indicate that the resize is taking place,
    /// such as updating a pointer cursor, during the resize. there is no guarantee that
    /// the device focus will return when the resize is completed. the edges parameter
    /// specifies how the surface should be resized, and is one of the values of the
    /// resize_edge enum. the compositor may use this information to update the surface
    /// position for example when dragging the top left corner. the compositor may also
    /// use this information to adapt its behavior, e.g. choose an appropriate cursor
    /// image.
    fn resize(&self, seat: WlSeat, serial: u32, edges: ResizeEdge) -> Result<(), SendError> {
        self.connection().borrow_mut().send(
            Request::Resize {
                seat,
                serial,
                edges,
            }
            .into_raw(self.id()),
        )
    }
    /// Set a maximum size for the window. the client can specify a maximum size so that
    /// the compositor does not try to configure the window beyond this size. the width
    /// and height arguments are in window geometry coordinates. see
    /// xdg_surface.set_window_geometry. values set in this way are double-buffered.
    /// they will get applied on the next commit. the compositor can use this
    /// information to allow or disallow different states like maximize or fullscreen
    /// and draw accurate animations. similarly, a tiling window manager may use this
    /// information to place and resize client windows in a more effective way. the
    /// client should not rely on the compositor to obey the maximum size. the
    /// compositor may decide to ignore the values set by the client and request a
    /// larger size. if never set, or a value of zero in the request, means that the
    /// client has no expected maximum size in the given dimension. as a result, a
    /// client wishing to reset the maximum size to an unspecified state can use zero
    /// for width and height in the request. requesting a maximum size to be smaller
    /// than the minimum size of a surface is illegal and will result in a protocol
    /// error. the width and height must be greater than or equal to zero. using
    /// strictly negative values for width and height will result in a protocol error.
    fn set_max_size(&self, width: i32, height: i32) -> Result<(), SendError> {
        self.connection()
            .borrow_mut()
            .send(Request::SetMaxSize { width, height }.into_raw(self.id()))
    }
    /// Set a minimum size for the window. the client can specify a minimum size so that
    /// the compositor does not try to configure the window below this size. the width
    /// and height arguments are in window geometry coordinates. see
    /// xdg_surface.set_window_geometry. values set in this way are double-buffered.
    /// they will get applied on the next commit. the compositor can use this
    /// information to allow or disallow different states like maximize or fullscreen
    /// and draw accurate animations. similarly, a tiling window manager may use this
    /// information to place and resize client windows in a more effective way. the
    /// client should not rely on the compositor to obey the minimum size. the
    /// compositor may decide to ignore the values set by the client and request a
    /// smaller size. if never set, or a value of zero in the request, means that the
    /// client has no expected minimum size in the given dimension. as a result, a
    /// client wishing to reset the minimum size to an unspecified state can use zero
    /// for width and height in the request. requesting a minimum size to be larger than
    /// the maximum size of a surface is illegal and will result in a protocol error.
    /// the width and height must be greater than or equal to zero. using strictly
    /// negative values for width and height will result in a protocol error.
    fn set_min_size(&self, width: i32, height: i32) -> Result<(), SendError> {
        self.connection()
            .borrow_mut()
            .send(Request::SetMinSize { width, height }.into_raw(self.id()))
    }
    /// Maximize the surface. after requesting that the surface should be maximized, the
    /// compositor will respond by emitting a configure event. whether this configure
    /// actually sets the window maximized is subject to compositor policies. the client
    /// must then update its content, drawing in the configured state. the client must
    /// also acknowledge the configure when committing the new content (see
    /// ack_configure). it is up to the compositor to decide how and where to maximize
    /// the surface, for example which output and what region of the screen should be
    /// used. if the surface was already maximized, the compositor will still emit a
    /// configure event with the "maximized" state. if the surface is in a fullscreen
    /// state, this request has no direct effect. it may alter the state the surface is
    /// returned to when unmaximized unless overridden by the compositor.
    fn set_maximized(&self) -> Result<(), SendError> {
        self.connection()
            .borrow_mut()
            .send(Request::SetMaximized {}.into_raw(self.id()))
    }
    /// Unmaximize the surface. after requesting that the surface should be unmaximized,
    /// the compositor will respond by emitting a configure event. whether this actually
    /// un-maximizes the window is subject to compositor policies. if available and
    /// applicable, the compositor will include the window geometry dimensions the
    /// window had prior to being maximized in the configure event. the client must then
    /// update its content, drawing it in the configured state. the client must also
    /// acknowledge the configure when committing the new content (see ack_configure).
    /// it is up to the compositor to position the surface after it was unmaximized;
    /// usually the position the surface had before maximizing, if applicable. if the
    /// surface was already not maximized, the compositor will still emit a configure
    /// event without the "maximized" state. if the surface is in a fullscreen state,
    /// this request has no direct effect. it may alter the state the surface is
    /// returned to when unmaximized unless overridden by the compositor.
    fn unset_maximized(&self) -> Result<(), SendError> {
        self.connection()
            .borrow_mut()
            .send(Request::UnsetMaximized {}.into_raw(self.id()))
    }
    /// Make the surface fullscreen. after requesting that the surface should be
    /// fullscreened, the compositor will respond by emitting a configure event. whether
    /// the client is actually put into a fullscreen state is subject to compositor
    /// policies. the client must also acknowledge the configure when committing the new
    /// content (see ack_configure). the output passed by the request indicates the
    /// client's preference as to which display it should be set fullscreen on. if this
    /// value is null, it's up to the compositor to choose which display will be used to
    /// map this surface. if the surface doesn't cover the whole output, the compositor
    /// will position the surface in the center of the output and compensate with with
    /// border fill covering the rest of the output. the content of the border fill is
    /// undefined, but should be assumed to be in some way that attempts to blend into
    /// the surrounding area (e.g. solid black). if the fullscreened surface is not
    /// opaque, the compositor must make sure that other screen content not part of the
    /// same surface tree (made up of subsurfaces, popups or similarly coupled surfaces)
    /// are not visible below the fullscreened surface.
    fn set_fullscreen(&self, output: Option<WlOutput>) -> Result<(), SendError> {
        self.connection()
            .borrow_mut()
            .send(Request::SetFullscreen { output }.into_raw(self.id()))
    }
    /// Make the surface no longer fullscreen. after requesting that the surface should
    /// be unfullscreened, the compositor will respond by emitting a configure event.
    /// whether this actually removes the fullscreen state of the client is subject to
    /// compositor policies. making a surface unfullscreen sets states for the surface
    /// based on the following: * the state(s) it may have had before becoming
    /// fullscreen * any state(s) decided by the compositor * any state(s) requested by
    /// the client while the surface was fullscreen the compositor may include the
    /// previous window geometry dimensions in the configure event, if applicable. the
    /// client must also acknowledge the configure when committing the new content (see
    /// ack_configure).
    fn unset_fullscreen(&self) -> Result<(), SendError> {
        self.connection()
            .borrow_mut()
            .send(Request::UnsetFullscreen {}.into_raw(self.id()))
    }
    /// Request that the compositor minimize your surface. there is no way to know if
    /// the surface is currently minimized, nor is there any way to unset minimization
    /// on this surface. if you are looking to throttle redrawing when minimized, please
    /// instead use the wl_surface.frame event for this, as this will also work with
    /// live previews on windows in alt-tab, expose or similar compositor features.
    fn set_minimized(&self) -> Result<(), SendError> {
        self.connection()
            .borrow_mut()
            .send(Request::SetMinimized {}.into_raw(self.id()))
    }
}