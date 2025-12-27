use core::arch::asm;
use core::mem::offset_of;

use ftl_types::error::ErrorCode;

/// Context of a thread.
pub struct Thread {
    pub sstatus: u64,
    pub sepc: u64,
    pub sp: u64,
    pub ra: u64,
    pub gp: u64,
    pub tp: u64,
    pub a0: u64,
    pub a1: u64,
    pub a2: u64,
    pub a3: u64,
    pub a4: u64,
    pub a5: u64,
    pub a6: u64,
    pub a7: u64,
    pub s0: u64,
    pub s1: u64,
    pub s2: u64,
    pub s3: u64,
    pub s4: u64,
    pub s5: u64,
    pub s6: u64,
    pub s7: u64,
    pub s8: u64,
    pub s9: u64,
    pub s10: u64,
    pub s11: u64,
    pub t0: u64,
    pub t1: u64,
    pub t2: u64,
    pub t3: u64,
    pub t4: u64,
    pub t5: u64,
    pub t6: u64,
}

impl Thread {
    pub fn new_idle() -> Self {
        let mut sstatus: u64;
        unsafe {
            asm!("csrr {0}, sstatus", out(reg) sstatus);
            sstatus |= 1 << 1; // Set SIE to enable interrupts
            sstatus |= 1 << 8; // Set SPP to go back to kernel mode
        }

        Self {
            sstatus,
            sepc: 0,
            sp: 0,
            ra: 0,
            gp: 0,
            tp: 0,
            a0: 0,
            a1: 0,
            a2: 0,
            a3: 0,
            a4: 0,
            a5: 0,
            a6: 0,
            a7: 0,
            s0: 0,
            s1: 0,
            s2: 0,
            s3: 0,
            s4: 0,
            s5: 0,
            s6: 0,
            s7: 0,
            s8: 0,
            s9: 0,
            s10: 0,
            s11: 0,
            t0: 0,
            t1: 0,
            t2: 0,
            t3: 0,
            t4: 0,
            t5: 0,
            t6: 0,
        }
    }

    pub fn new(pc: usize, sp: usize, arg: usize) -> Self {
        let mut sstatus: u64;
        unsafe {
            asm!("csrr {0}, sstatus", out(reg) sstatus);
            sstatus |= 1 << 8; // Set SPP to go back to kernel mode
        }

        Self {
            sstatus,
            sepc: pc as u64,
            sp: sp as u64,
            ra: 0,
            gp: 0,
            tp: 0,
            a0: arg as u64,
            a1: 0,
            a2: 0,
            a3: 0,
            a4: 0,
            a5: 0,
            a6: 0,
            a7: 0,
            s0: 0,
            s1: 0,
            s2: 0,
            s3: 0,
            s4: 0,
            s5: 0,
            s6: 0,
            s7: 0,
            s8: 0,
            s9: 0,
            s10: 0,
            s11: 0,
            t0: 0,
            t1: 0,
            t2: 0,
            t3: 0,
            t4: 0,
            t5: 0,
            t6: 0,
        }
    }

    pub fn set_syscall_result(&mut self, retval: Result<usize, ErrorCode>) {
        match retval {
            Ok(value) => {
                self.a0 = 0;
                self.a1 = value as u64;
            }
            Err(err) => {
                self.a0 = err as u64;
            }
        };
    }
}

pub fn switch_into_thread(thread: *mut Thread) -> ! {
    unsafe {
        // Fill CSRs.
        asm!("csrw sstatus, {}", in(reg) (*thread).sstatus);
        asm!("csrw sepc, {}", in(reg) (*thread).sepc);

        // Restore general-purpose registers.
        asm!(
            "ld ra, {ra_offset}(a0)",
            "ld sp, {sp_offset}(a0)",
            "ld gp, {gp_offset}(a0)",
            "ld tp, {tp_offset}(a0)",
            "ld t0, {t0_offset}(a0)",
            "ld t1, {t1_offset}(a0)",
            "ld t2, {t2_offset}(a0)",
            "ld s0, {s0_offset}(a0)",
            "ld s1, {s1_offset}(a0)",
            "ld a1, {a1_offset}(a0)",
            "ld a2, {a2_offset}(a0)",
            "ld a3, {a3_offset}(a0)",
            "ld a4, {a4_offset}(a0)",
            "ld a5, {a5_offset}(a0)",
            "ld a6, {a6_offset}(a0)",
            "ld a7, {a7_offset}(a0)",
            "ld s2, {s2_offset}(a0)",
            "ld s3, {s3_offset}(a0)",
            "ld s4, {s4_offset}(a0)",
            "ld s5, {s5_offset}(a0)",
            "ld s6, {s6_offset}(a0)",
            "ld s7, {s7_offset}(a0)",
            "ld s8, {s8_offset}(a0)",
            "ld s9, {s9_offset}(a0)",
            "ld s10, {s10_offset}(a0)",
            "ld s11, {s11_offset}(a0)",
            "ld t3, {t3_offset}(a0)",
            "ld t4, {t4_offset}(a0)",
            "ld t5, {t5_offset}(a0)",
            "ld t6, {t6_offset}(a0)",
            "ld a0, {a0_offset}(a0)",
            "sret",
            in ("a0") thread as usize,
            ra_offset = const offset_of!(Thread, ra),
            sp_offset = const offset_of!(Thread, sp),
            gp_offset = const offset_of!(Thread, gp),
            tp_offset = const offset_of!(Thread, tp),
            t0_offset = const offset_of!(Thread, t0),
            t1_offset = const offset_of!(Thread, t1),
            t2_offset = const offset_of!(Thread, t2),
            s0_offset = const offset_of!(Thread, s0),
            s1_offset = const offset_of!(Thread, s1),
            a0_offset = const offset_of!(Thread, a0),
            a1_offset = const offset_of!(Thread, a1),
            a2_offset = const offset_of!(Thread, a2),
            a3_offset = const offset_of!(Thread, a3),
            a4_offset = const offset_of!(Thread, a4),
            a5_offset = const offset_of!(Thread, a5),
            a6_offset = const offset_of!(Thread, a6),
            a7_offset = const offset_of!(Thread, a7),
            s2_offset = const offset_of!(Thread, s2),
            s3_offset = const offset_of!(Thread, s3),
            s4_offset = const offset_of!(Thread, s4),
            s5_offset = const offset_of!(Thread, s5),
            s6_offset = const offset_of!(Thread, s6),
            s7_offset = const offset_of!(Thread, s7),
            s8_offset = const offset_of!(Thread, s8),
            s9_offset = const offset_of!(Thread, s9),
            s10_offset = const offset_of!(Thread, s10),
            s11_offset = const offset_of!(Thread, s11),
            t3_offset = const offset_of!(Thread, t3),
            t4_offset = const offset_of!(Thread, t4),
            t5_offset = const offset_of!(Thread, t5),
            t6_offset = const offset_of!(Thread, t6),
            options(noreturn)
        );
    }
}
