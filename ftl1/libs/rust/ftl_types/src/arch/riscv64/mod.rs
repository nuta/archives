const PTE_V: u64 = 1 << 0;
const PTE_R: u64 = 1 << 1;
const PTE_W: u64 = 1 << 2;
const PTE_X: u64 = 1 << 3;

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
        Self(PTE_V)
    }

    pub const fn read(mut self) -> Self {
        self.0 |= PTE_R;
        self
    }

    pub const fn write(mut self) -> Self {
        self.0 |= PTE_W;
        self
    }

    pub const fn execute(mut self) -> Self {
        self.0 |= PTE_X;
        self
    }

    pub const fn bits(self) -> u64 {
        self.0
    }
}
