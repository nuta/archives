use core::arch::asm;
use core::arch::naked_asm;
use core::mem::MaybeUninit;

use ftl_utils::byte_size::ByteSize;
use ftl_utils::static_assert;

use super::serial;
use super::vmspace::KERNEL_BASE_ADDR;
use super::vmspace::TMP_PAGE_TABLE;
use crate::address::PAddr;
use crate::arch::INTERRUPT_CONTROLLER;
use crate::arch::arm64::exception::default_vector_table;
use crate::arch::arm64::timer;
use crate::arch::paddr2vaddr;
use crate::boot::BootInfo;
use crate::cpuvar::CpuId;

const NUM_CPUS_MAX: usize = 4;

#[repr(align(4096))]
struct Stack(#[allow(unused)] [u8; KERNEL_STACK_SIZE]);

#[unsafe(link_section = ".data")]
static KERNEL_STACKS: MaybeUninit<[Stack; NUM_CPUS_MAX]> = MaybeUninit::uninit();

pub const KERNEL_STACK_SIZE: usize = 512 * ByteSize::KiB;
pub const SP_BOTTOM_SHIFT: u32 = KERNEL_STACK_SIZE.trailing_zeros();

// Memory Attribute Indirection Register.
//
// - Attr0 = 0x00 (Device-nGnRnE)
// - Attr1 = 0xff (Normal Write-Back)
const MAIR_EL1: u64 = 0xff;

// Translation Control Register.
//
// - 39-bit VA: T0SZ=25, T1SZ=25
// - 4KB granule: TG0=0b00, TG1=0b10
// - Inner Shareable: SH0=0b11, SH1=0b11
// - Write-Back: ORGN0/1=0b01, IRGN0/1=0b01
const TCR_EL1: u64 = 0xb5193519;

// System Control Register.
// Enable MMU, data cache, and instruction cache.
const SCTLR_EL1: u64 = 1 << 0 | 1 << 2 | 1 << 12;

static_assert!(
    KERNEL_STACK_SIZE.count_ones() == 1,
    "Kernel stack size must be a power of two"
);

#[unsafe(no_mangle)]
#[unsafe(link_section = ".text.boot")]
#[unsafe(naked)]
unsafe extern "C" fn _start() -> ! {
    naked_asm!(
        // Symbol addresses (adrp/adr instructions) are in physical memory address,
        // not virtual address because pc is in physical, and these do PC-relative
        // addressing.
        "mov x29, xzr",
        "mov x30, xzr",

        // Read the CPU ID (lower 8 bits of MPIDR_EL1).
        "mrs x1, mpidr_el1",
        "and x1, x1, #0xff",

        // If cpu_id >= NUM_CPUS_MAX, do not use this CPU.
        "mov x10, {num_cpus_max}",
        "cmp x1, x10",
        "b.ge 1f",

        // Configure MMU and caches.
        "ldr x10, ={mair_el1}",
        "msr mair_el1, x10",
        "ldr x10, ={tcr_el1}",
        "msr tcr_el1, x10",

        // Load the temporary kernel page table (TMP_PAGE_TABLE).
        "adrp x10, {tmp_pgtbl}",
        "add x10, x10, :lo12:{tmp_pgtbl}",
        "msr ttbr0_el1, x10",
        "msr ttbr1_el1, x10",

        // Invalidate the TLB and the instruction cache.
        "dsb sy",
        "tlbi vmalle1",
        "dsb sy",
        "ic ialluis",
        "isb",

        // Enable MMU.
        "mrs x10, sctlr_el1",
        "ldr x11, ={sctlr_el1}",
        "orr x10, x10, x11",
        "msr sctlr_el1, x10",
        "isb",

        // x20 = KERNEL_BASE_ADDR (to paddr2vaddr)
        "mov x20, {kernel_base_addr}",

        // Initialize the stack pointer.
        "add x10, x1, #1",                     // x10 = cpu_id + 1
        "lsl x10, x10, {sp_bottom_shift}",     // x10 = (cpu_id + 1) * KERNEL_STACK_SIZE
        "adrp x11, {kernel_stacks}",           // Get the paddr of KERNEL_STACKS
        "add x11, x11, :lo12:{kernel_stacks}", // x11 = paddr2vaddr(KERNEL_STACKS)
        "add x11, x11, x10",                   // x11 = KERNEL_STACKS + offset
        "orr x11, x11, x20",                   // x11 = paddr2vaddr(x11)
        "sub x11, x11, #16",                   // Guarantee the SP_BOTTOM_SHIFT trick work
        "mov sp, x11",                         // sp = x11

        // Save the kernel sp for handlers.
        "msr tpidr_el1, x11",

        // Jump into rust_boot.
        "adr x10, {rust_boot}", // x10 = paddr of rust_boot
        "add x10, x10, x20",    // x10 = paddr2vaddr(rust_boot)
        "br x10",

        "1:",
        "wfi",
        "b 1b",
        num_cpus_max = const NUM_CPUS_MAX,
        mair_el1 = const MAIR_EL1,
        tcr_el1 = const TCR_EL1,
        sctlr_el1 = const SCTLR_EL1,
        tmp_pgtbl = sym TMP_PAGE_TABLE,
        kernel_stacks = sym KERNEL_STACKS,
        sp_bottom_shift = const SP_BOTTOM_SHIFT,
        kernel_base_addr = const KERNEL_BASE_ADDR.as_usize(),
        rust_boot = sym rust_boot,
    );
}

extern "C" fn rust_boot(_unused: u64, cpu_id: u64) -> ! {
    assert_eq!(cpu_id, 0, "only CPU 0 is supported");

    unsafe {
        asm!("msr vbar_el1, {}", in(reg) &raw const default_vector_table as u64);
    }

    serial::init();

    let cpu = CpuId::new(cpu_id as usize);
    INTERRUPT_CONTROLLER.init_per_cpu(cpu);

    // QEMU virt machine puts the DTB at 0x40000000.
    let dtb = paddr2vaddr(PAddr::new(0x4000_0000));
    timer::init();

    crate::boot::boot(BootInfo { dtb, cpu_id: cpu });
}
