use alloc::vec::Vec;
use core::fmt;

use ftl_types::syscall::SYS_LOG_WRITE;
pub use log::debug;
pub use log::error;
pub use log::info;
pub use log::trace;
pub use log::warn;

use crate::arch::get_vsyscall_page;
use crate::syscall::syscall2;

#[macro_export]
macro_rules! debug_warn {
    ($($arg:tt)+) => {
        if cfg!(debug_assertions) {
            ::log::warn!($($arg)+);
        }
    };
}

struct Writer {
    buf: spin::Mutex<Vec<u8>>,
}

static LOGGER: Writer = Writer::new();

impl Writer {
    const fn new() -> Writer {
        Writer {
            buf: spin::Mutex::new(Vec::new()),
        }
    }

    fn write_str(&self, s: &str) {
        let mut buf = self.buf.lock();
        for b in s.bytes() {
            buf.push(b);
            if b == b'\n' {
                let old_buf = core::mem::replace(&mut *buf, Vec::with_capacity(128));

                // Do not hold the lock while writing to the console. This
                // logger could be shared between multiple apps/threads,
                // and the kernel may switch to another app/thread.
                drop(buf);

                sys_log_write(&old_buf);
                buf = self.buf.lock();
            }
        }
    }
}

fn sys_log_write(buf: &[u8]) {
    let _ = syscall2(SYS_LOG_WRITE, buf.as_ptr() as usize, buf.len());
}

/// A `Writer` wrapper. This is necessary because fmt::Write expects
/// `write_str` to be `&mut self`, but `Writer::write_str` takes
/// `&self`.
struct WriterWrapper;

impl fmt::Write for WriterWrapper {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        LOGGER.write_str(s);
        Ok(())
    }
}

impl log::Log for Writer {
    fn enabled(&self, _metadata: &log::Metadata) -> bool {
        true
    }

    fn log(&self, record: &log::Record) {
        if !self.enabled(record.metadata()) {
            return;
        }

        let vsyscall_page = get_vsyscall_page();
        let name = vsyscall_page.name();

        let level = match record.level() {
            log::Level::Error => "ERROR",
            log::Level::Warn => "WARN",
            log::Level::Info => "INFO",
            log::Level::Debug => "DEBUG",
            log::Level::Trace => "TRACE",
        };

        let color = match record.level() {
            log::Level::Error => "\x1b[91m",
            log::Level::Warn => "\x1b[33m",
            log::Level::Info => "\x1b[96m",
            log::Level::Debug | log::Level::Trace => "\x1b[0m",
        };

        const RESET_COLOR: &str = "\x1b[0m";

        let print_target = !record.target().starts_with(name);

        use core::fmt::Write;
        writeln!(
            WriterWrapper,
            "[{name:<12}] {color}{level:<6}{RESET_COLOR} {target_prefix}{target}{target_suffix}{args}",
            target_prefix = if print_target { "[" } else { "" },
            target = if print_target { record.target() } else { "" },
            target_suffix = if print_target {  "] " } else { "" },
            args = record.args()
        )
        .unwrap();
    }

    fn flush(&self) {
        // Console writing is immediate, no buffering to flush
    }
}

/// Initialize the logger. This should be called once at the start of the program.
pub fn init() {
    if log::set_logger(&LOGGER).is_ok() {
        log::set_max_level(if cfg!(debug_assertions) {
            log::LevelFilter::Trace
        } else {
            log::LevelFilter::Info
        });
    }
}
