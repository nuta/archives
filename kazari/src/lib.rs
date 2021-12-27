#![no_std]
#![allow(unused)] // FIXME:
#![allow(clippy::from_over_into)]

#[macro_use]
extern crate std;
extern crate alloc;
#[macro_use]
extern crate log;
#[cfg(test)]
#[macro_use]
extern crate pretty_assertions;

pub mod canvas;
pub mod client;
mod icons;
pub mod server;
pub mod standalone;
mod utils;
pub mod wl;

pub use utils::mmap;

use core::cmp::{max, min};
use core::fmt;

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum PixelFormat {
    RGBA8888,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Position {
    pub x: usize,
    pub y: usize,
}

impl Position {
    pub const fn new(x: usize, y: usize) -> Position {
        Position { x, y }
    }

    pub const fn zero() -> Position {
        Position::new(0, 0)
    }

    pub fn clamp(self, min_pos: Position, max_pos: Position) -> Position {
        Position::new(
            max(min_pos.x, min(self.x, max_pos.x)),
            max(min_pos.y, min(self.y, max_pos.y)),
        )
    }
}

impl core::ops::Add for Position {
    type Output = Position;
    fn add(self, rhs: Position) -> Self::Output {
        Position::new(self.x + rhs.x, self.y + rhs.y)
    }
}

impl core::ops::Sub for Position {
    type Output = Position;
    fn sub(self, rhs: Position) -> Self::Output {
        Position::new(self.x - rhs.x, self.y - rhs.y)
    }
}

impl fmt::Display for Position {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "({}, {})", self.x, self.y)
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct RectSize {
    pub width: usize,
    pub height: usize,
}

impl RectSize {
    pub const fn new(width: usize, height: usize) -> RectSize {
        RectSize { width, height }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct RectArea {
    pub pos: Position,
    pub size: RectSize,
}

impl RectArea {
    pub const fn new(pos: Position, size: RectSize) -> RectArea {
        RectArea { pos, size }
    }

    pub fn overlapping(self, other: RectArea) -> Option<RectArea> {
        let p1 = self.pos;
        let p2 = other.pos;
        let s1 = self.size;
        let s2 = other.size;
        let non_overlapping = p1.x + s1.width < p2.x
            || p2.x + s2.width < p1.x
            || p1.y + s1.height < p2.y
            || p2.y + s2.height < p1.y;
        if non_overlapping {
            return None;
        }

        let start_x = max(p1.x, p2.x);
        let start_y = max(p1.y, p2.y);
        let end_x = min(p1.x + s1.width, p2.x + s2.width);
        let end_y = min(p1.y + s1.height, p2.y + s2.height);

        Some(RectArea::new(
            Position::new(start_x, start_y),
            RectSize::new(end_x - start_x, end_y - start_y),
        ))
    }

    pub fn contains_position(&self, pos: Position) -> bool {
        self.pos.x <= pos.x
            && pos.x < self.pos.x + self.size.width
            && self.pos.y <= pos.y
            && pos.y < self.pos.y + self.size.height
    }
}

#[derive(Debug, Clone, Copy)]
pub enum Color {
    Rgba8888 { r: u8, g: u8, b: u8, a: u8 },
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum FontFamily {
    Default,
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum FontStyle {
    Regular,
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum FontSize {
    Normal,
}

#[derive(Debug, Clone, Copy)]
pub struct FillStyle {
    pub color: Color,
}

#[derive(Debug, Clone, Copy)]
pub struct BorderStyle {
    pub color: Color,
    pub width: usize,
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn rect_area_overlapping() {
        let a = RectArea::new(Position::new(20, 20), RectSize::new(10, 10));
        let b = RectArea::new(Position::new(19, 20), RectSize::new(10, 10));
        assert_eq!(
            a.overlapping(b),
            Some(RectArea::new(Position::new(20, 20), RectSize::new(9, 10)))
        );
    }
}
