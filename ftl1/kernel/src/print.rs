use core::cmp::min;

use ftl_types::error::ErrorCode;

use crate::arch;
use crate::isolation::IsolationSlice;
use crate::shared_ref::SharedRef;
use crate::syscall::SyscallResult;
use crate::thread::Thread;

/// The console output writer.
///
/// This is an internal implementation detail of the `print!` and `println!`
/// macros. You should use those macros, not this struct directly.
pub struct Printer;

impl core::fmt::Write for Printer {
    fn write_str(&mut self, s: &str) -> core::fmt::Result {
        let bytes = s.as_bytes();
        arch::console_write(bytes);
        Ok(())
    }
}

/// Prints a string and a newline.
#[macro_export]
macro_rules! println {
    () => {{
        #[allow(unused_imports)]
        use core::fmt::Write;
        writeln!($crate::print::Printer).ok();
    }};
    ($fmt:expr) => {{
        #[allow(unused_imports)]
        use core::fmt::Write;
        writeln!($crate::print::Printer, $fmt).ok();
    }};
    ($fmt:expr, $($arg:tt)*) => {{
        #[allow(unused_imports)]
        use core::fmt::Write;
        writeln!($crate::print::Printer, $fmt, $($arg)*).ok();
    }};
}

#[derive(Clone, Copy, PartialEq, Eq, Ord, PartialOrd)]
pub enum LogLevel {
    Error,
    Warn,
    Info,
    Debug,
    Trace,
}

#[macro_export]
macro_rules! log {
    ($level:expr, $($arg:tt)+) => {{
        use $crate::print::LogLevel;

        const RESET_COLOR: &str = "\x1b[0m";

        if cfg!(debug_assertions) || $level <= LogLevel::Info {
            let (color, level_str) = match $level {
                LogLevel::Error => ("\x1b[91m", "ERR"),
                LogLevel::Warn =>  ("\x1b[33m", "WARN"),
                LogLevel::Info =>  ("\x1b[96m", "INFO"),
                LogLevel::Debug => ("\x1b[0m", "DEBUG"),
                LogLevel::Trace => ("\x1b[0m", "TRACE"),
            };

            $crate::println!(
                "[kernel      ] {}{:6}{} {}",
                color,
                level_str,
                RESET_COLOR,
                format_args!($($arg)+)
            );
        }
    }};
}

#[macro_export]
macro_rules! error {
    ($($arg:tt)+) => { $crate::log!($crate::print::LogLevel::Error, $($arg)+) }
}

#[macro_export]
macro_rules! warn {
    ($($arg:tt)+) => { $crate::log!($crate::print::LogLevel::Warn, $($arg)+) }
}

#[macro_export]
macro_rules! info {
    ($($arg:tt)+) => { $crate::log!($crate::print::LogLevel::Info, $($arg)+) }
}

#[macro_export]
macro_rules! debug {
    ($($arg:tt)+) => { $crate::log!($crate::print::LogLevel::Debug, $($arg)+) }
}

#[macro_export]
macro_rules! trace {
    ($($arg:tt)+) => { $crate::log!($crate::print::LogLevel::Trace, $($arg)+) }
}

#[macro_export]
macro_rules! debug_warn {
    ($($arg:tt)+) => {
        if cfg!(debug_assertions) {
            $crate::warn!($($arg)+);
        }
    };
}

/// `log_write` system call handler.
pub fn sys_log_write(
    current: &SharedRef<Thread>,
    a0: usize,
    a1: usize,
) -> Result<SyscallResult, ErrorCode> {
    let slice = IsolationSlice::from_raw(a0, a1);

    let mut tmp = [0u8; 512];
    let mut remaining = slice.len();
    let mut offset = 0;
    while remaining > 0 {
        let chunk_len = min(remaining, tmp.len());
        let buf = &mut tmp[..chunk_len];

        slice.read_bytes(current.process().isolation(), offset, buf)?;
        arch::console_write(buf);

        offset += chunk_len;
        remaining -= chunk_len;
    }

    Ok(SyscallResult::Return(0))
}
