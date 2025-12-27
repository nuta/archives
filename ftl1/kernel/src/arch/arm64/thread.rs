use core::arch::asm;
use core::mem::offset_of;

use ftl_types::error::ErrorCode;

#[derive(Default)]
pub struct Thread {
    pub spsr: u64,
    pub elr: u64,
    pub sp: u64,
    pub tpidr_el0: u64,
    pub x0: u64,
    pub x1: u64,
    pub x2: u64,
    pub x3: u64,
    pub x4: u64,
    pub x5: u64,
    pub x6: u64,
    pub x7: u64,
    pub x8: u64,
    pub x9: u64,
    pub x10: u64,
    pub x11: u64,
    pub x12: u64,
    pub x13: u64,
    pub x14: u64,
    pub x15: u64,
    pub x16: u64,
    pub x17: u64,
    pub x18: u64,
    pub x19: u64,
    pub x20: u64,
    pub x21: u64,
    pub x22: u64,
    pub x23: u64,
    pub x24: u64,
    pub x25: u64,
    pub x26: u64,
    pub x27: u64,
    pub x28: u64,
    pub x29: u64,
    pub x30: u64,
}

impl Thread {
    pub fn new_idle() -> Self {
        let mut spsr: u64;
        unsafe {
            asm!("mrs {}, spsr_el1", out(reg) spsr);
            spsr |= 1 << 7; // Enable IRQ
            spsr &= !0b1111; // Clear mode bits
            spsr |= 0x5; // EL1h
        }

        Self {
            spsr,
            ..Default::default()
        }
    }

    pub fn new(pc: usize, sp: usize, arg: usize) -> Self {
        let mut spsr: u64;
        unsafe {
            asm!("mrs {}, spsr_el1", out(reg) spsr);
            spsr |= 1 << 7; // Enable IRQ
            spsr &= !0b1111; // Clear mode bits
            spsr |= 0x4; // EL1t
        }

        Self {
            spsr,
            elr: pc as u64,
            sp: sp as u64,
            x0: arg as u64,
            ..Default::default()
        }
    }

    pub fn set_syscall_result(&mut self, retval: Result<usize, ErrorCode>) {
        match retval {
            Ok(value) => {
                self.x0 = 0;
                self.x1 = value as u64;
            }
            Err(err) => {
                self.x0 = err as u64;
            }
        };
    }
}

pub fn switch_into_thread(thread: *mut Thread) -> ! {
    unsafe {
        asm!("msr spsr_el1, {}", in(reg) (*thread).spsr);
        asm!("msr elr_el1, {}", in(reg) (*thread).elr);
        asm!("msr tpidr_el0, {}", in(reg) (*thread).tpidr_el0);

        asm!(
            // Update SP_EL0.
            "msr spsel, #0",
            "ldr x1, [x0, {sp_offset}]",
            "mov sp, x1",

            "ldr x1, [x0, {x1_offset}]",
            "ldr x2, [x0, {x2_offset}]",
            "ldr x3, [x0, {x3_offset}]",
            "ldr x4, [x0, {x4_offset}]",
            "ldr x5, [x0, {x5_offset}]",
            "ldr x6, [x0, {x6_offset}]",
            "ldr x7, [x0, {x7_offset}]",
            "ldr x8, [x0, {x8_offset}]",
            "ldr x9, [x0, {x9_offset}]",
            "ldr x10, [x0, {x10_offset}]",
            "ldr x11, [x0, {x11_offset}]",
            "ldr x12, [x0, {x12_offset}]",
            "ldr x13, [x0, {x13_offset}]",
            "ldr x14, [x0, {x14_offset}]",
            "ldr x15, [x0, {x15_offset}]",
            "ldr x16, [x0, {x16_offset}]",
            "ldr x17, [x0, {x17_offset}]",
            "ldr x18, [x0, {x18_offset}]",
            "ldr x19, [x0, {x19_offset}]",
            "ldr x20, [x0, {x20_offset}]",
            "ldr x21, [x0, {x21_offset}]",
            "ldr x22, [x0, {x22_offset}]",
            "ldr x23, [x0, {x23_offset}]",
            "ldr x24, [x0, {x24_offset}]",
            "ldr x25, [x0, {x25_offset}]",
            "ldr x26, [x0, {x26_offset}]",
            "ldr x27, [x0, {x27_offset}]",
            "ldr x28, [x0, {x28_offset}]",
            "ldr x29, [x0, {x29_offset}]",
            "ldr x30, [x0, {x30_offset}]",
            "ldr x0, [x0, {x0_offset}]",
            "eret",
            in("x0") thread as usize,
            sp_offset = const offset_of!(Thread, sp),
            x0_offset = const offset_of!(Thread, x0),
            x1_offset = const offset_of!(Thread, x1),
            x2_offset = const offset_of!(Thread, x2),
            x3_offset = const offset_of!(Thread, x3),
            x4_offset = const offset_of!(Thread, x4),
            x5_offset = const offset_of!(Thread, x5),
            x6_offset = const offset_of!(Thread, x6),
            x7_offset = const offset_of!(Thread, x7),
            x8_offset = const offset_of!(Thread, x8),
            x9_offset = const offset_of!(Thread, x9),
            x10_offset = const offset_of!(Thread, x10),
            x11_offset = const offset_of!(Thread, x11),
            x12_offset = const offset_of!(Thread, x12),
            x13_offset = const offset_of!(Thread, x13),
            x14_offset = const offset_of!(Thread, x14),
            x15_offset = const offset_of!(Thread, x15),
            x16_offset = const offset_of!(Thread, x16),
            x17_offset = const offset_of!(Thread, x17),
            x18_offset = const offset_of!(Thread, x18),
            x19_offset = const offset_of!(Thread, x19),
            x20_offset = const offset_of!(Thread, x20),
            x21_offset = const offset_of!(Thread, x21),
            x22_offset = const offset_of!(Thread, x22),
            x23_offset = const offset_of!(Thread, x23),
            x24_offset = const offset_of!(Thread, x24),
            x25_offset = const offset_of!(Thread, x25),
            x26_offset = const offset_of!(Thread, x26),
            x27_offset = const offset_of!(Thread, x27),
            x28_offset = const offset_of!(Thread, x28),
            x29_offset = const offset_of!(Thread, x29),
            x30_offset = const offset_of!(Thread, x30),
            options(noreturn)
        );
    }
}
