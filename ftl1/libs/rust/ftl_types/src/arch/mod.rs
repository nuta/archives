#[cfg(all(target_os = "none", target_arch = "riscv64"))]
mod riscv64;

#[cfg(all(target_os = "none", target_arch = "riscv64"))]
pub use riscv64::*;

#[cfg(all(target_os = "none", target_arch = "aarch64"))]
mod arm64;

#[cfg(all(target_os = "none", target_arch = "aarch64"))]
pub use arm64::*;

#[cfg(not(target_os = "none"))]
mod host;

#[cfg(not(target_os = "none"))]
pub use host::*;
