//! Type-safe wrappers for big/little endian fields.

/// Big endian fields.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
#[repr(transparent)]
pub struct BE<T> {
    value: T,
}

impl<T> BE<T> {
    pub fn new(value: T) -> Self {
        Self { value }
    }
}

impl BE<u16> {
    pub fn get(&self) -> u16 {
        u16::from_be(self.value)
    }

    pub fn set(&mut self, value: u16) {
        self.value = value.to_be();
    }
}

impl From<u16> for BE<u16> {
    fn from(value: u16) -> Self {
        Self {
            value: value.to_be(),
        }
    }
}

impl From<BE<u16>> for u16 {
    fn from(value: BE<u16>) -> Self {
        u16::from_be(value.value)
    }
}

impl BE<u32> {
    pub fn get(&self) -> u32 {
        u32::from_be(self.value)
    }

    pub fn set(&mut self, value: u32) {
        self.value = value.to_be();
    }
}

impl From<u32> for BE<u32> {
    fn from(value: u32) -> Self {
        Self {
            value: value.to_be(),
        }
    }
}

impl From<BE<u32>> for u32 {
    fn from(value: BE<u32>) -> Self {
        u32::from_be(value.value)
    }
}

impl BE<u64> {
    pub fn get(&self) -> u64 {
        u64::from_be(self.value)
    }

    pub fn set(&mut self, value: u64) {
        self.value = value.to_be();
    }
}

impl From<u64> for BE<u64> {
    fn from(value: u64) -> Self {
        Self {
            value: value.to_be(),
        }
    }
}

impl From<BE<u64>> for u64 {
    fn from(value: BE<u64>) -> Self {
        u64::from_be(value.value)
    }
}

/// Little endian fields.
#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
#[repr(transparent)]
pub struct LE<T> {
    value: T,
}

impl<T> LE<T> {
    pub fn new(value: T) -> Self {
        Self { value }
    }
}

impl LE<u16> {
    pub fn get(&self) -> u16 {
        u16::from_le(self.value)
    }

    pub fn set(&mut self, value: u16) {
        self.value = value.to_le();
    }
}

impl From<u16> for LE<u16> {
    fn from(value: u16) -> Self {
        Self {
            value: value.to_le(),
        }
    }
}

impl From<LE<u16>> for u16 {
    fn from(value: LE<u16>) -> Self {
        u16::from_le(value.value)
    }
}

impl LE<u32> {
    pub fn get(&self) -> u32 {
        u32::from_le(self.value)
    }

    pub fn set(&mut self, value: u32) {
        self.value = value.to_le();
    }
}

impl From<u32> for LE<u32> {
    fn from(value: u32) -> Self {
        Self {
            value: value.to_le(),
        }
    }
}

impl From<LE<u32>> for u32 {
    fn from(value: LE<u32>) -> Self {
        u32::from_le(value.value)
    }
}

impl LE<u64> {
    pub fn get(&self) -> u64 {
        u64::from_le(self.value)
    }

    pub fn set(&mut self, value: u64) {
        self.value = value.to_le();
    }
}

impl From<u64> for LE<u64> {
    fn from(value: u64) -> Self {
        Self {
            value: value.to_le(),
        }
    }
}

impl From<LE<u64>> for u64 {
    fn from(value: LE<u64>) -> Self {
        u64::from_le(value.value)
    }
}
