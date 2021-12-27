macro_rules! define_csr_register {
    ($struct_name:ident, $reg_name:literal) => {
        #[derive(Debug)]
        pub struct $struct_name(usize);

        impl $struct_name {
            #[allow(unused)]
            pub unsafe fn from_raw(value: usize) -> $struct_name {
                $struct_name(value)
            }

            #[allow(unused)]
            pub unsafe fn read() -> $struct_name {
                let mut value;
                cfg_if!{
                    if #[cfg(target_arch = "riscv64")] {
                        asm!(concat!("csrr {0}, ", $reg_name), out(reg) value);
                    } else {
                        // A quick workaround to suppress "use of
                        // "possibly-uninitialized variable" in make lint.
                        value = 0;
                    }
                }
                $struct_name(value)
            }

            #[allow(unused)]
            pub unsafe fn write(&self) {
                #[cfg(target_arch = "riscv64")]
                asm!(concat!("csrw ", $reg_name, ", {0}"), in(reg) self.0);
            }
        }

    };
}

///  Supervisor Address Translation and Protection Register.
pub mod satp {
    define_csr_register!(Satp, "satp");
}

/// The address mode (`A` field) in Physical Memory Protection Configuration Registers.
#[repr(u8)]
pub enum PmpAddressMode {
    TopOfRange = 1,
}

///  Physical Memory Protection Configuration Register 0.
pub mod pmpcfg0 {
    use bit_field::BitField;

    use super::PmpAddressMode;

    define_csr_register!(PmpCfg0, "pmpcfg0");

    impl PmpCfg0 {
        pub fn new() -> PmpCfg0 {
            PmpCfg0(0)
        }

        pub fn set_readable(&mut self, value: bool) -> &mut Self {
            self.0.set_bit(0, value);
            self
        }

        pub fn set_writable(&mut self, value: bool) -> &mut Self {
            self.0.set_bit(1, value);
            self
        }

        pub fn set_executable(&mut self, value: bool) -> &mut Self {
            self.0.set_bit(2, value);
            self
        }

        pub fn set_address_mode(&mut self, value: PmpAddressMode) -> &mut Self {
            self.0.set_bits(3..=4, value as u8 as usize);
            self
        }
    }
}

///  Physical Memory Protection Address Register 0.
pub mod pmpaddr0 {
    define_csr_register!(PmpAddr0, "pmpaddr0");
}

/// Machine Exception Program Counter.
pub mod mepc {
    define_csr_register!(Mepc, "mepc");
}

/// Machine Status Register.
pub mod mstatus {
    use bit_field::BitField;

    define_csr_register!(Mstatus, "mstatus");

    /// Machine Previous Privilege Mode.
    pub enum Mpp {
        Supervisor = 1,
    }

    impl Mstatus {
        pub fn set_mpp(&mut self, value: Mpp) -> &mut Self {
            self.0.set_bits(11..=12, value as usize);
            self
        }
    }
}

///  Machine Trap Delegation Registers (Exceptions).
pub mod medeleg {
    define_csr_register!(Medeleg, "medeleg");
}

/// Supervisor Trap Vector Base Address Register.
pub mod stvec {
    use bit_field::BitField;

    define_csr_register!(Stvec, "stvec");

    impl Stvec {
        pub fn new(addr: usize) -> Stvec {
            debug_assert!(
                addr.get_bits(0..=1) == 0,
                "lower 2 bits in stvec's addr must be 0"
            );

            // SAFETY: The raw value are checked in assertions.
            unsafe { Stvec::from_raw(addr) }
        }
    }
}
