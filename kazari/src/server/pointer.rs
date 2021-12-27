use crate::Position;

#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum PointerButtonState {
    Pressed,
    Released,
}

#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum PointerButton {
    Left,
    Right,
}

#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum PointerMove {
    Relative { y_offset: usize, x_offset: usize },
    Absolute { new_pos: Position },
}
