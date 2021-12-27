use crate::canvas::Canvas;
use crate::client::ClientError;
use crate::utils::mmap::MMap;
use crate::wl::protocols::client::wl_seat::WlSeatExt;
use crate::wl::protocols::client::wl_surface::WlSurfaceExt;
use crate::wl::protocols::client::xdg_toplevel::XdgToplevelExt;
use crate::wl::protocols::common::wl_keyboard::WlKeyboard;
use crate::wl::protocols::common::wl_pointer::WlPointer;
use crate::wl::protocols::common::wl_seat::WlSeat;
use crate::wl::protocols::common::wl_surface::WlSurface;
use crate::wl::protocols::common::xdg_surface::XdgSurface;
use crate::wl::protocols::common::xdg_toplevel::XdgToplevel;
use crate::wl::Interface;
use crate::PixelFormat;
use crate::{Position, RectArea, RectSize};
use alloc::boxed::Box;
use alloc::rc::Rc;
use alloc::string::ToString;

pub struct Window {
    canvas: Canvas,
    wl_seat: WlSeat,
    wl_keyboard: WlKeyboard,
    wl_pointer: WlPointer,
    _xdg_surface: XdgSurface,
    xdg_toplevel: XdgToplevel,
    window_surface: WlSurface,
}

impl Window {
    pub fn new(
        wl_seat: WlSeat,
        wl_keyboard: WlKeyboard,
        wl_pointer: WlPointer,
        xdg_surface: XdgSurface,
        xdg_toplevel: XdgToplevel,
        window_surface: WlSurface,
        mmap: Rc<MMap>,
        size: RectSize,
        format: PixelFormat,
    ) -> Window {
        Window {
            wl_seat,
            wl_keyboard,
            wl_pointer,
            _xdg_surface: xdg_surface,
            xdg_toplevel,
            window_surface,
            canvas: Canvas::new(Box::new(mmap), size.width, size.height, format),
        }
    }

    pub fn surface(&self) -> &WlSurface {
        &self.window_surface
    }

    pub fn canvas(&mut self) -> &Canvas {
        &self.canvas
    }

    pub fn canvas_mut(&mut self) -> &mut Canvas {
        &mut self.canvas
    }

    pub fn damage(&mut self, area: RectArea) -> Result<(), ClientError> {
        let Position { x, y } = area.pos;
        let RectSize { width, height } = area.size;
        self.window_surface
            .damage(x as i32, y as i32, width as i32, height as i32)
            .map_err(ClientError::SendError)
    }

    pub fn commit(&mut self) -> Result<(), ClientError> {
        self.window_surface.commit().map_err(ClientError::SendError)
    }

    /// Set the window title.
    pub fn set_title(&mut self, title: &str) -> Result<(), ClientError> {
        self.xdg_toplevel
            .set_title(title.to_string())
            .map_err(ClientError::SendError)
    }

    // Enables the keyboard input.
    pub fn enable_keyboard(&mut self) -> Result<(), ClientError> {
        self.wl_seat
            .get_keyboard(self.wl_keyboard.as_new_id())
            .map_err(ClientError::SendError)
    }

    // Enables the mouse input.
    pub fn enable_pointer(&mut self) -> Result<(), ClientError> {
        self.wl_seat
            .get_pointer(self.wl_pointer.as_new_id())
            .map_err(ClientError::SendError)
    }
}
