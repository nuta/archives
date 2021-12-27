use crate::canvas::Canvas;
use crate::icons::{XpmColor, XpmImage};
use crate::server::client::Client;
use crate::server::keyboard::{KeyCode, KeyState};
use crate::server::objects::wl_surface::SurfaceObject;
use crate::server::objects::xdg_toplevel::XdgToplevelObject;
use crate::server::WINDOW_TITLE_BAR_HEIGHT;
use crate::wl::protocols::server::wl_keyboard::WlKeyboardExt;
use crate::Color;
use crate::FillStyle;
use crate::Position;
use crate::RectArea;
use crate::RectSize;
use crate::{FontFamily, FontSize, FontStyle};
use alloc::rc::Rc;

use core::cell::{Ref, RefCell, RefMut};

pub enum LayerContent {
    Window {
        surface: Rc<RefCell<SurfaceObject>>,
        xdg_toplevel: Rc<RefCell<XdgToplevelObject>>,
    },
    SolidColor {
        color: Color,
    },
    XpmImage {
        image: &'static XpmImage,
    },
}

pub struct Layer {
    /// The top-left position in the screen.
    position: Position,
    /// The size of the content.
    size: RectSize,
    /// The surface to be drawn.
    content: LayerContent,
    /// Whether the layer is draggable.
    draggable: bool,
}

impl Layer {
    pub fn new(
        position: Position,
        size: RectSize,
        content: LayerContent,
        draggable: bool,
    ) -> Layer {
        Layer {
            position,
            size,
            content,
            draggable,
        }
    }

    pub fn position(&self) -> Position {
        self.position
    }

    pub fn size(&self) -> RectSize {
        self.size
    }

    pub fn area(&self) -> RectArea {
        RectArea::new(self.position, self.size)
    }

    pub fn content(&self) -> &LayerContent {
        &self.content
    }

    pub fn draggable(&self) -> bool {
        self.draggable
    }

    pub fn move_to(&mut self, new_pos: Position) {
        self.position = new_pos;
    }

    pub fn render(&self, dst: &mut Canvas, damaged_area: RectArea, active: bool) {
        // trace!(
        //     "render a layer: area={:?}, damaged={:?}, overlap={:?}",
        //     self.area(),
        //     damaged_area,
        //     self.area().overlapping(damaged_area)
        // );
        if let Some(overlapping_area) = self.area().overlapping(damaged_area) {
            let local_area =
                RectArea::new(overlapping_area.pos - self.position, overlapping_area.size);
            match self.content() {
                LayerContent::Window {
                    surface,
                    xdg_toplevel,
                } => {
                    let layer_area = self.area();
                    let title_bar_area = RectArea::new(
                        layer_area.pos,
                        RectSize::new(layer_area.size.width, WINDOW_TITLE_BAR_HEIGHT),
                    );
                    let contents_area = RectArea::new(
                        Position::new(layer_area.pos.x, layer_area.pos.y + WINDOW_TITLE_BAR_HEIGHT),
                        RectSize::new(
                            layer_area.size.width,
                            layer_area.size.height - WINDOW_TITLE_BAR_HEIGHT,
                        ),
                    );

                    // The title bar.
                    if let Some(overlapping_area) = title_bar_area.overlapping(damaged_area) {
                        let title_bar_color = if active {
                            Color::Rgba8888 {
                                r: 70,
                                g: 60,
                                b: 70,
                                a: 255,
                            }
                        } else {
                            Color::Rgba8888 {
                                r: 150,
                                g: 140,
                                b: 150,
                                a: 255,
                            }
                        };
                        dst.draw_rect(
                            overlapping_area.pos,
                            overlapping_area.size,
                            Some(FillStyle {
                                color: title_bar_color,
                            }),
                            None,
                        );
                        dst.draw_text(
                            &xdg_toplevel.borrow().title,
                            FontSize::Normal,
                            FontFamily::Default,
                            FontStyle::Regular,
                            self.position + Position::new(10, 0),
                            Color::Rgba8888 {
                                r: 255,
                                g: 255,
                                b: 255,
                                a: 255,
                            },
                        );
                    }

                    // Window contents.
                    if let Some(overlapping_area) = contents_area.overlapping(damaged_area) {
                        let local_area = RectArea::new(
                            overlapping_area.pos - contents_area.pos,
                            overlapping_area.size,
                        );
                        if let Some(buffer) = &surface.borrow().buffer {
                            dst.draw_canvas(
                                overlapping_area.pos,
                                &buffer.borrow().canvas,
                                local_area,
                            );
                        }
                    }
                }
                LayerContent::SolidColor { color } => {
                    // trace!(
                    //     "solid: {:?} {:?}",
                    //     overlapping_area.pos,
                    //     overlapping_area.size
                    // );
                    dst.draw_rect(
                        overlapping_area.pos,
                        overlapping_area.size,
                        Some(FillStyle { color: *color }),
                        None,
                    );
                }
                LayerContent::XpmImage { image } => {
                    for y in local_area.pos.y..(local_area.pos.y + local_area.size.height) {
                        for x in local_area.pos.x..(local_area.pos.x + local_area.size.width) {
                            let pos = Position::new(
                                overlapping_area.pos.x + x - local_area.pos.x,
                                overlapping_area.pos.y + y - local_area.pos.y,
                            );
                            let color = match image.bitmap[y * image.width + x] {
                                XpmColor::Black => Color::Rgba8888 {
                                    r: 0,
                                    g: 0,
                                    b: 0,
                                    a: 255,
                                },
                                XpmColor::White => Color::Rgba8888 {
                                    r: 255,
                                    g: 255,
                                    b: 255,
                                    a: 255,
                                },
                                XpmColor::Transparent => dst.pixel_color(pos),
                            };
                            dst.draw_pixel(pos, color);
                        }
                    }
                }
            }
        }
    }
}
