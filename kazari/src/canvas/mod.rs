use crate::{
    BorderStyle, Color, FillStyle, FontFamily, FontSize, FontStyle, PixelFormat, Position,
    RectArea, RectSize,
};
use alloc::boxed::Box;
use embedded_graphics::{
    mono_font::{ascii::Font9x18, MonoTextStyle},
    pixelcolor::Rgb888,
    primitives::Primitive,
    primitives::PrimitiveStyleBuilder,
    primitives::Rectangle,
    text::Text,
    Drawable,
};

mod inner;

impl Into<Rgb888> for Color {
    fn into(self) -> Rgb888 {
        match self {
            Color::Rgba8888 { r, g, b, .. } => Rgb888::new(r, g, b),
        }
    }
}

impl Into<embedded_graphics::geometry::Point> for Position {
    fn into(self) -> embedded_graphics::geometry::Point {
        embedded_graphics::geometry::Point::new(self.x as i32, self.y as i32)
    }
}

impl Into<embedded_graphics::geometry::Size> for RectSize {
    fn into(self) -> embedded_graphics::geometry::Size {
        embedded_graphics::geometry::Size::new(self.width as u32, self.height as u32)
    }
}

pub trait CanvasBuffer {
    fn as_ref(&self) -> &[u8];
    fn as_mut(&mut self) -> &mut [u8];
}

pub struct Canvas {
    buffer: Box<dyn CanvasBuffer>,
    width: usize,
    height: usize,
    format: PixelFormat,
}

impl Canvas {
    pub fn new(
        buffer: Box<dyn CanvasBuffer>,
        width: usize,
        height: usize,
        format: PixelFormat,
    ) -> Canvas {
        Canvas {
            buffer,
            width,
            height,
            format,
        }
    }

    pub fn buffer(&self) -> &[u8] {
        (*self.buffer).as_ref()
    }

    pub fn buffer_mut(&mut self) -> &mut [u8] {
        (*self.buffer).as_mut()
    }

    pub fn width(&self) -> usize {
        self.width
    }

    pub fn height(&self) -> usize {
        self.height
    }

    pub fn size(&self) -> RectSize {
        RectSize::new(self.width, self.height)
    }

    pub fn format(&self) -> PixelFormat {
        self.format
    }

    pub fn bytes_per_pixel(&self) -> usize {
        match self.format {
            PixelFormat::RGBA8888 => 4,
        }
    }

    pub fn pixel_offset(&self, pos: Position) -> usize {
        match self.format {
            PixelFormat::RGBA8888 => pos.y as usize * self.width * 4 + pos.x as usize * 4,
        }
    }

    pub fn pixel(&self, pos: Position) -> &[u8] {
        &self.buffer()[self.pixel_offset(pos)..]
    }

    pub fn pixel_mut(&mut self, pos: Position) -> &mut [u8] {
        let offset = self.pixel_offset(pos);
        &mut self.buffer_mut()[offset..]
    }

    pub fn pixel_color(&self, pos: Position) -> Color {
        let pixel = self.pixel(pos);
        Color::Rgba8888 {
            r: pixel[0],
            g: pixel[1],
            b: pixel[2],
            a: pixel[3],
        }
    }

    pub fn draw_pixel(&mut self, pos: Position, color: Color) {
        if pos.y as usize >= self.height || pos.x as usize >= self.width {
            warn!("draw_pixel: out of bounds {}", pos);
            return;
        }

        match (&self.format, color) {
            (PixelFormat::RGBA8888, Color::Rgba8888 { r, g, b, a }) => {
                let pixel = self.pixel_mut(pos);
                pixel[0] = r;
                pixel[1] = g;
                pixel[2] = b;
                pixel[3] = a;
            }
        }
    }

    pub fn draw_rect(
        &mut self,
        pos: Position,
        size: RectSize,
        fill: Option<FillStyle>,
        border: Option<BorderStyle>,
    ) {
        let mut builder = PrimitiveStyleBuilder::new();
        if let Some(FillStyle { color }) = fill {
            builder = builder.fill_color(color.into());
        }

        if let Some(BorderStyle { color, width }) = border {
            builder = builder.stroke_color(color.into());
            builder = builder.stroke_width(width as u32);
        }

        Rectangle::new(pos.into(), size.into())
            .into_styled(builder.build())
            .draw(self)
            .ok();
    }

    pub fn draw_text(
        &mut self,
        text: &str,
        _size: FontSize,
        _font: FontFamily,
        _style: FontStyle,
        pos: Position,
        color: Color,
    ) {
        Text::new(text, (pos + Position::new(0, 18)/* FIXME: */).into())
            .into_styled(MonoTextStyle::new(Font9x18, color.into()))
            .draw(self)
            .ok();
    }

    pub fn draw_canvas(&mut self, pos: Position, canvas: &Canvas, area: RectArea) {
        assert!(self.format() == canvas.format());
        let bytes_per_pixel = self.bytes_per_pixel();
        for y_offset in 0..area.size.height {
            for x_offset in 0..area.size.width {
                let dst = self.pixel_mut(Position::new(pos.x + x_offset, pos.y + y_offset));
                let src = canvas.pixel(Position::new(area.pos.x + x_offset, area.pos.y + y_offset));
                dst[..bytes_per_pixel].copy_from_slice(&src[..bytes_per_pixel]);
            }
        }
    }
}
