use core::arch::asm;
use core::arch::naked_asm;
use core::mem::MaybeUninit;

use ftl_utils::byte_size::ByteSize;
use ftl_utils::static_assert;

use super::INTERRUPT_CONTROLLER;
use super::sbi;
use super::timer;
use super::trap::trap_handler;
use super::vmspace::KERNEL_BASE_ADDR;
use super::vmspace::PPN_SHIFT;
use super::vmspace::SATP_MODE_SV48;
use super::vmspace::TMP_PAGE_TABLE;
use super::vmspace::paddr2vaddr;
use crate::address::PAddr;
use crate::boot::BootInfo;
use crate::cpuvar::CpuId;

const NUM_CPUS_MAX: usize = 4;

#[repr(align(4096))]
struct Stack(#[allow(unused)] [u8; KERNEL_STACK_SIZE]);

#[unsafe(link_section = ".data")]
static KERNEL_STACKS: MaybeUninit<[Stack; NUM_CPUS_MAX]> = MaybeUninit::uninit();

pub const KERNEL_STACK_SIZE: usize = 512 * ByteSize::KiB;
pub const SP_BOTTOM_SHIFT: u32 = KERNEL_STACK_SIZE.trailing_zeros();

static_assert!(
    KERNEL_STACK_SIZE.count_ones() == 1,
    "Kernel stack size must be a power of two"
);

// The kernel entrypoint for RISC-V machines. We expect Linux's RISC-V boot
// requirements:
//
//   - a0: THe hartid of this CPU.
//   - a1: The address of the device tree blob.
#[unsafe(no_mangle)]
#[unsafe(link_section = ".text.boot")]
#[unsafe(naked)]
unsafe extern "C" fn riscv64_boot(hartid: u64, dtb: PAddr) -> ! {
    naked_asm!(
        // Note: Don't modify a0, a1 registers here: they are used as arguments to
        //       rust_boot.
        //
        // Also, symbol addresses (lla instructions) are in physical memory address,
        // not virtual address because pc is in physical, and LLA does PC-relative
        // addressing.
        "mv ra, zero",
        "mv fp, zero",

        // If hartid >= NUM_CPUS_MAX, do not use this CPU.
        "li t0, {num_cpus_max}",
        "bge a0, t0, 1f",

        // Load a temporary kernel page table (TMP_PAGE_TABLE).
        "lla t0, {tmp_pgtbl}",
        "srli t0, t0, {ppn_shift}",
        "li t1, {satp_mode}",
        "or t0, t0, t1",
        "sfence.vma",
        "csrw satp, t0",
        "sfence.vma",

        // t1 = KERNEL_BASE_ADDR (to paddr2vaddr)
        "li t1, {kernel_base_addr}",

        // Initialize the stack pointer.
        "addi t0, a0, 1",                 // t0 = hartid + 1
        "slli t0, t0, {sp_bottom_shift}", // t0 = (hartid + 1) * KERNEL_STACK_SIZE
        "lla sp, {kernel_stacks}",        // Get the paddr of KERNEL_STACKS
        "add  sp, sp, t0",                // sp = KERNEL_STACKS + offset
        "or sp, sp, t1",                  // sp = paddr2vaddr(sp)
        "addi sp, sp, -16",               // Guarantee the SP_BOTTOM_SHIFT trick work

        // Initialize sscratch with kernel stack
        "csrw sscratch, sp",

        // Jump into rust_boot.
        "lla t0, {rust_boot}", // Get the paddr of rust_boot
        "or t0, t0, t1",       // t0 = paddr2vaddr(t0)
        "jr t0",

        // A halt loop for CPUs that are not allowed to boot.
        "1:",
        "wfi",
        "j 1b",
        num_cpus_max = const NUM_CPUS_MAX,
        tmp_pgtbl = sym TMP_PAGE_TABLE,
        ppn_shift = const PPN_SHIFT,
        kernel_stacks = sym KERNEL_STACKS,
        sp_bottom_shift = const SP_BOTTOM_SHIFT,
        kernel_base_addr = const KERNEL_BASE_ADDR.as_usize(),
        satp_mode = const SATP_MODE_SV48,
        rust_boot = sym rust_boot,
    );
}

#[unsafe(no_mangle)]
extern "C" fn rust_boot(hartid: u64, dtb: PAddr) -> ! {
    unsafe {
        asm!("csrw stvec, {}", in(reg) trap_handler as *const () as usize);

        let mut sie: u64;
        asm!("csrr {}, sie", out(reg) sie);
        sie |= 1 << 1; // SSIE: supervisor-level software interrupts
        sie |= 1 << 5; // STIE: supervisor-level timer interrupts
        sie |= 1 << 9; // SEIE: supervisor-level external interrupts
        asm!("csrw sie, {}", in(reg) sie);
    }

    assert_eq!(hartid, 0, "only hart 0 is supported");

    let cpu_id = CpuId::new(hartid as usize);
    INTERRUPT_CONTROLLER.init_per_cpu(cpu_id);

    // FIXME: QEMU virt machine specific.
    timer::set_frequency(10_000_000);

    // Disable the timer.
    sbi::set_timer(0xffff_ffff_ffff_ffff);

    crate::boot::boot(BootInfo {
        cpu_id,
        dtb: paddr2vaddr(dtb),
    });
}
