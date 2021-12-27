use crate::{canvas::Canvas, Color, Position};
use embedded_graphics::{
    draw_target::DrawTarget, geometry::OriginDimensions, pixelcolor::Rgb888, prelude::*,
};

impl OriginDimensions for Canvas {
    fn size(&self) -> Size {
        Size::new(self.width as u32, self.height as u32)
    }
}

impl DrawTarget for Canvas {
    type Color = Rgb888;
    type Error = core::convert::Infallible;

    fn draw_iter<I>(&mut self, pixels: I) -> Result<(), Self::Error>
    where
        I: IntoIterator<Item = Pixel<Self::Color>>,
    {
        for Pixel(point, color) in pixels.into_iter() {
            self.draw_pixel(
                Position::new(point.x as usize, point.y as usize),
                Color::Rgba8888 {
                    r: color.r(),
                    g: color.g(),
                    b: color.b(),
                    a: 255,
                },
            );
        }

        Ok(())
    }
}
