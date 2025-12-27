const PTE_VALID: u64 = 1 << 0;
const PTE_AF: u64 = 1 << 10;
const PTE_AP_RW: u64 = 0 << 7;
const PTE_UXN: u64 = 1 << 54;
const PTE_PXN: u64 = 1 << 53;

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
        Self(PTE_VALID | PTE_AF)
    }

    pub const fn read(self) -> Self {
        self
    }

    pub const fn write(mut self) -> Self {
        self.0 |= PTE_AP_RW;
        self
    }

    pub const fn execute(mut self) -> Self {
        self.0 &= !PTE_UXN;
        self.0 &= !PTE_PXN;
        self
    }

    pub const fn bits(self) -> u64 {
        self.0
    }
}
