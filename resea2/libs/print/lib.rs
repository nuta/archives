//! The printing library.
//!
//! This crate provides print-related macros like `print!` and `println!`.
//!
//! # Required Functions
//! This library internally depends on `extern "C" fn printchar(c: u8)`.
#![cfg_attr(not(test), no_std)]

extern "C" {
    fn printchar(c: u8);
}

pub struct Printer;

impl core::fmt::Write for Printer {
    fn write_char(&mut self, c: char) -> core::fmt::Result {
        // SAFETY: Printing a character should not be dangerous.
        unsafe {
            printchar(c as u8);
        }
        Ok(())
    }

    fn write_str(&mut self, s: &str) -> core::fmt::Result {
        for c in s.chars() {
            // SAFETY: Printing a character should not be dangerous.
            unsafe {
                printchar(c as u8);
            }
        }
        Ok(())
    }
}

/// Prints a string.
#[macro_export]
macro_rules! print {
    ($($arg:tt)*) => {{
        #![allow(unused_import)]
        use core::fmt::Write;
        let _ = write!($crate::Printer, "{}", format_args!($($arg)*));
    }};
}

/// Prints a string and a newline.
#[macro_export]
macro_rules! println {
    ($fmt:expr) => {{
        $crate::print!(concat!($fmt, "\n"));
    }};
    ($fmt:expr, $($arg:tt)*) => {{
        $crate::print!(concat!($fmt, "\n"), $($arg)*);
    }};
}
