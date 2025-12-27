#[derive(Clone, Copy, Debug)]
pub struct PageAttrs(u64);

impl PageAttrs {
    pub const fn from_usize(value: usize) -> Self {
        Self(value as u64)
    }

    pub const fn to_usize(self) -> usize {
        self.0 as usize
    }

    pub const fn valid() -> Self {
        todo!()
    }

    pub const fn read(self) -> Self {
        todo!()
    }

    pub const fn write(self) -> Self {
        todo!()
    }

    pub const fn execute(self) -> Self {
        todo!()
    }
}
