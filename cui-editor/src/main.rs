#[macro_use]
extern crate log;

mod editor;
mod buffer;
mod window;
mod terminal;
mod color;
mod logger;
mod plugins;

pub type Result<T> = std::result::Result<T, failure::Error>;

fn main() {
    logger::init();

    std::panic::set_hook(Box::new(|info| {
        error!("{}", info);

        // Print the backtrace in reverse order.
        use std::fmt::Write;
        let mut buf = String::new();
        let trace = backtrace::Backtrace::new();
        for (i, frame) in trace.frames().iter().rev().enumerate() {
            use termion::color::{Fg, LightRed};
            use termion::style::{Bold, Reset};

            write!(buf, "\n{}{:4}:{} ", Bold, i, Reset).ok();
            for symbol in frame.symbols().iter() {
                if let Some(name) = symbol.name() {
                    write!(buf, "{}{}{}{}", Bold, Fg(LightRed), name, Reset).ok();
                }

                if let (Some(file), Some(line)) = (symbol.filename(), symbol.lineno()) {
                    write!(buf, "\n        at {}:{}", file.display(), line).ok();
                }
            }
        }

        error!("{}", buf);
    }));

    let mut editor = editor::Editor::new();
    plugins::load_plugins(&mut editor);
    editor.open_file("test.txt", false).expect("failed to open test.txt");

    use crate::editor::Key;
    editor.register_key_binding(Key::Ctrl('a'), "move.begin");
    editor.register_key_binding(Key::Ctrl('e'), "move.end");
    editor.register_key_binding(Key::Ctrl('f'), "finder.open");
    editor.register_key_binding(Key::CtrlDown, "cursor.add_below");

    editor.run();
}
