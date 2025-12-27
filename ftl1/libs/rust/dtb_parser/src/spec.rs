//! <https://devicetree-specification.readthedocs.io/en/stable/flattened-format.html>

use ftl_utils::endianess::BE;

pub const FDT_MAGIC: u32 = 0xd00dfeed;

/// `struct fdt_header`.
///
/// <https://devicetree-specification.readthedocs.io/en/stable/flattened-format.html#header>
#[repr(C)]
pub struct Header {
    pub magic: BE<u32>,
    pub totalsize: BE<u32>,
    pub off_dt_struct: BE<u32>,
    pub off_dt_strings: BE<u32>,
    pub off_mem_rsvmap: BE<u32>,
    pub version: BE<u32>,
    pub last_comp_version: BE<u32>,
    pub boot_cpuid_phys: BE<u32>,
    /// In bytes.
    pub size_dt_strings: BE<u32>,
    /// In bytes.
    pub size_dt_struct: BE<u32>,
}

// https://devicetree-specification.readthedocs.io/en/stable/flattened-format.html#lexical-structure
pub const FDT_BEGIN_NODE: u32 = 1;
pub const FDT_END_NODE: u32 = 2;
pub const FDT_PROP: u32 = 3;
pub const FDT_NOP: u32 = 4;
pub const FDT_END: u32 = 9;
