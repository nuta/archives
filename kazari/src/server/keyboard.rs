#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum KeyState {
    Pressed,
    Released,
}

#[derive(Debug, Copy, Clone, PartialEq, Eq)]
#[repr(u32)]
pub enum KeyCode {
    // TODO:
    KeyA = 0x41,
    KeyB = 0x42,
    KeyC = 0x43,
    KeyD = 0x44,

    LeftCtrl,
    LeftAlt,
    LeftShift,
    LeftSuper,
    RightCtrl,
    RightAlt,
    RightShift,
    RightSuper,
    CapsLock,
    Backspace,
    Delete,
    Insert,
    ArrowLeft,
    ArrowRight,
    ArrowUp,
    ArrowDown,
    PageUp,
    PageDown,
    Home,
    End,
    Esc,
    Tab,
    Space,
    Enter,
}
