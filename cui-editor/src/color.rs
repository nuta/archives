use termion;
use termion::color::Rgb;
use lazy_static::lazy_static;
use std::collections::HashMap;

#[derive(PartialEq, Eq, Hash, Copy, Clone)]
pub enum Color {
    ActiveStatusLine,
    InActiveStatusLine,
    LineNumber,
    ActionBox
}

lazy_static! {
    static ref SOLARIZED: HashMap<Color, Rgb> = {
        let mut palette = HashMap::new();

        palette.insert(Color::ActiveStatusLine,   Rgb(0x85, 0x99, 0x00));
        palette.insert(Color::InActiveStatusLine, Rgb(0x83, 0x94, 0x96));
        palette.insert(Color::LineNumber,         Rgb(0x07, 0x36, 0x42));
        palette.insert(Color::ActionBox,          Rgb(0x93, 0xa1, 0xa1));

        palette
    };
}

pub fn color(color: Color) -> termion::color::Rgb {
    SOLARIZED.get(&color).unwrap().clone()
}