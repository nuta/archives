pub trait EndianExt {
    fn to_ne(&self) -> Self;
    fn to_he(&self) -> Self;
}

impl EndianExt for u16 {
    fn to_ne(&self) -> Self {
        (self << 8) | (self >> 8)
    }

    fn to_he(&self) -> Self {
        (self << 8) | (self >> 8)
    }
}

impl EndianExt for u32 {
    fn to_ne(&self) -> Self {
        (self >> 24) |
        ((self >> 8) & 0x0000ff00u32) |
        ((self << 8) & 0x00ff0000u32) |
        (self << 24)
    }

    fn to_he(&self) -> Self {
        (self >> 24) |
        ((self >> 8) & 0x0000ff00u32) |
        ((self << 8) & 0x00ff0000u32) |
        (self << 24)
    }
}
