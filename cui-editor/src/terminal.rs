use crate::editor::{Event, Key};
use crate::window::{Cursor, Point, Window};
use crate::color::{color, Color};
use log::trace;
use termion::input::TermRead;
use termion::raw::{IntoRawMode, RawTerminal};
use syntect::easy::HighlightLines;
use syntect::parsing::SyntaxSet;
use syntect::highlighting::ThemeSet;
use std::io::Write;
use std::sync::mpsc::Sender;
use std::sync::{Arc, Mutex};

pub struct Terminal {
    tx: Sender<Event>,
    stdout: RawTerminal<std::io::Stdout>,
    width: usize,
    height: usize,
    syntax_set: SyntaxSet,
    theme_set: ThemeSet,
}

/// Returns `(width, height)`.
#[inline]
fn get_terminal_size() -> (usize, usize) {
    let (w, h) = termion::terminal_size().unwrap();
    (w as usize, h as usize)
}

#[inline]
fn goto(y: usize, x: usize) -> termion::cursor::Goto {
    termion::cursor::Goto(x as u16 + 1, y as u16 + 1)
}

#[inline]
fn num_digits(mut num: usize) -> usize {
    let mut digits = 1;
    while num / 10 > 0 {
        num /= 10;
        digits += 1;
    }

    digits
}

impl Terminal {
    pub fn new(tx: Sender<Event>) -> Terminal {
        let stdout = std::io::stdout().into_raw_mode().unwrap();
        let (width, height) = get_terminal_size();
        let syntax_set = SyntaxSet::load_defaults_newlines();
        let theme_set = ThemeSet::load_defaults();
        Terminal {
            stdout,
            width,
            height,
            tx,
            syntax_set,
            theme_set,
        }
    }

    pub fn force_render(&mut self, current_win: &Arc<Mutex<Window>>, windows: &[Arc<Mutex<Window>>]) {
        let (width, height) = get_terminal_size();
        self.width = width;
        self.height = height;

        self.render(current_win, windows, true);
    }

    pub fn render(&mut self, current_win: &Arc<Mutex<Window>>, windows: &[Arc<Mutex<Window>>], force_render: bool) {
        use std::fmt::Write;

        let statusline_height = 1;
        let mut screen = String::new();
        let current_win_id = current_win.lock().unwrap().id();

        let mut x_base = 0;
        let mut current_x_base = None;
        let mut current_win_lineno_width = None;
        let mut current_win_editbox_width = None;
        let mut current_win_editbox_height = None;
        let mut current_actionbox_cursor_pos = None;

        let syntax = self.syntax_set.find_syntax_by_extension("rs").unwrap();
        let mut highlighter = HighlightLines::new(syntax, &self.theme_set.themes["base16-ocean.dark"]);

        // Render each window.
        for (i, win_lock) in windows.iter().enumerate() {
            let mut win = win_lock.lock().unwrap();

            // Compute the x-axis length of the window.
            let win_width = if i == 0 {
                self.width / windows.len() + self.width % windows.len()
            } else {
                self.width / windows.len()
            };

            let (actionbox_rendered, actionbox_height) =
                if let Some(action_box) = win.action_box() {
                    // Render actionbox.
                    let actionbox_height_max = 15;
                    let max_height = std::cmp::min(actionbox_height_max, self.height - statusline_height);
                    let (lines, cursor) = action_box.borrow().render(win_width, max_height);
                    trace!("lines: {}, height: {}", lines.len(), self.height);
                    let editbox_height = self.height - statusline_height - lines.len();

                    let mut rendered = String::new();
                    for (i, line) in lines.iter().enumerate() {
                        write!(
                            rendered,
                            "{}{}",
                            goto(editbox_height + i, 0),
                            line
                        ).ok();
                    }

                    if current_win_id == win.id() {
                        let cursor_pos = Cursor::Normal(
                            Point::new(editbox_height + cursor.y, x_base + cursor.x));
                        current_actionbox_cursor_pos = Some(cursor_pos);
                    }

                    (rendered, lines.len())
                } else {
                    // Action box is not opened in this window.
                    ("".to_owned(), 0)
                };

            let editbox_height = self.height - statusline_height - actionbox_height;
            let lineno_digits = num_digits(win.buffer().lock().unwrap().num_lines());
            let lineno_width = lineno_digits + 2;
            let editbox_width = win_width - lineno_width - 1;

            win.update_offset(editbox_width, editbox_height);
            trace!("win_width={}, editbox_height={}, win_offset={}, cursors={:?}",
                win_width, editbox_height, win.offset(), win.cursors());

            let needs_redraw = win.needs_redraw() || win.cursors().len() > 1;
            win.reset_needs_redraw();

            let buffer = win.buffer().lock().unwrap();

            // This must come after `win.update_offset()` since it updates `needs_redraw`.
            if needs_redraw || force_render {
                // Render editbox.
                for line in 0..editbox_height {
                    let y = win.offset().y + line;
                    if let Some(content) = buffer.line(y, win.offset().x, editbox_width) {
                        use syntect::util::as_24_bit_terminal_escaped;
                        use syntect::highlighting::Style;
                        let ranges: Vec<(Style, &str)> = highlighter.highlight(&content, &self.syntax_set);
                        let colored_content = as_24_bit_terminal_escaped(&ranges, false);

                        let lineno = win.offset().y + line + 1;
                        write!(
                            screen,
                            "{}{}{} {}{} {} {}{}",
                            goto(line, x_base),
                            termion::style::Reset,
                            termion::color::Bg(color(Color::LineNumber)),
                            // Padding before the lineno
                            " ".repeat(lineno_digits - num_digits(lineno)),
                            lineno,
                            termion::style::Reset,
                            colored_content,
                            // Padding: clears the previous content.
                            " ".repeat(editbox_width - content.len())
                        ).ok();
                    } else {
                        // A line after the end of file.
                        write!(
                            screen,
                            "{}{}{} {} {} {}",
                            goto(line, x_base),
                            termion::style::Reset,
                            termion::style::Invert,
                            " ".repeat(lineno_digits),
                            termion::style::Reset,
                            // Padding: clears the previous content.
                            " ".repeat(editbox_width)
                        ).ok();
                    }
                }
            }

            // Render action box.
            write!(
                screen,
                "{}{}{}",
                termion::style::Reset,
                goto(editbox_height, x_base),
                actionbox_rendered
            ).ok();

            // Render status line.
            if win.id() == current_win_id {
                write!(screen, "{}",
                    termion::color::Bg(color(Color::ActiveStatusLine))).ok();
            } else {
                write!(screen, "{}",
                    termion::color::Bg(color(Color::InActiveStatusLine))).ok();
            };

            let padding_len = win_width - buffer.filepath().len();
            write!(
                screen,
                "{}{}{}{}{}{}",
                goto(editbox_height + actionbox_height, x_base),
                termion::style::Bold,
                buffer.filepath(),
                termion::style::NoBold,
                " ".repeat(padding_len),
                termion::style::Reset,
            ).ok();

            if win.id() == current_win_id {
                current_x_base = Some(x_base);
                current_win_lineno_width = Some(lineno_width);
                current_win_editbox_width = Some(editbox_width);
                current_win_editbox_height = Some(editbox_height);
            }

            // The beginning x-axis offset of the next window.
            x_base += win_width;
        }

        // Move the cursor.
        if let Some(cursor) = current_actionbox_cursor_pos {
            match cursor {
                Cursor::Normal(Point { x, y }) => {
                    write!(screen, "{}", goto(y, x)).ok();
                }
            }
        } else {
            let current_win = current_win.lock().unwrap();
            // Render multiple cursors.
            for cursor in current_win.cursors().iter().skip(1) {
                match *cursor.borrow() {
                    Cursor::Normal(Point { x: cur_x, y: cur_y }) => {
                        let Point { x: win_x, y: win_y } = *current_win.offset();

                        if cur_y < win_y || cur_y > win_x + current_win_editbox_height.unwrap() {
                            // The cursor is out of window.
                            continue;
                        }

                        if cur_x < win_x || cur_x > win_x + current_win_editbox_width.unwrap() {
                            // The cursor is out of window.
                            continue;
                        }

                        let y = cur_y - win_y;
                        let x = current_x_base.unwrap()
                            + current_win_lineno_width.unwrap()
                            + 1 // padding between lineno and editbox
                            + cur_x
                            - win_x;
                        write!(screen, "{}{} ", goto(y, x), termion::style::Invert).ok();
                    }
                }
            }

            // Render the main cursor.
            let main_cursor = current_win.cursors()[0].borrow().clone();
            match main_cursor {
                Cursor::Normal(Point { x: cur_x, y: cur_y }) => {
                    let Point { x: win_x, y: win_y } = current_win.offset();
                    let y = cur_y - win_y;
                    let x = current_x_base.unwrap()
                        + current_win_lineno_width.unwrap()
                        + 1 // padding between lineno and editbox
                        + cur_x
                        - win_x;
                    write!(screen, "{}", goto(y, x)).ok();
                }
            }
        }

        self.draw_screen(&screen);
    }

    fn draw_screen(&mut self, screen: &str) {
        write!(
            self.stdout,
            "{}{}{}",
            termion::cursor::Hide,
            screen,
            termion::cursor::Show
        ).ok();
        self.stdout.flush().ok();
    }

    pub fn quit(&mut self) {
        // Switch back into the main screen.
        write!(
            self.stdout,
            "{}{}{}",
            termion::clear::All,
            termion::screen::ToMainScreen,
            termion::style::Reset
        ).ok();
        self.stdout.flush().ok();
    }

    pub fn start(&mut self) {
        let user_input_tx = self.tx.clone();
        std::thread::spawn(move || {
            user_input_handler(user_input_tx);
        });

        let allowed_signals = [signal_hook::SIGWINCH];
        let signals = signal_hook::iterator::Signals::new(&allowed_signals).unwrap();
        let signal_tx = self.tx.clone();
        std::thread::spawn(move || {
            for signal in signals.forever() {
                match signal {
                    signal_hook::SIGWINCH => {
                        trace!("received SIGWINCH");
                        signal_tx.send(Event::NeedsRedraw).ok();
                    }
                    _ => {
                        trace!("received unexpected signal: {}", signal);
                    }
                }
            }
        });

        // Switch into the alternate screen.
        write!(self.stdout, "{}", termion::screen::ToAlternateScreen).ok();
    }
}

fn termion_key2input_key(key: termion::event::Key) -> Option<Key> {
    match key {
        termion::event::Key::Backspace  => Some(Key::Backspace),
        termion::event::Key::Delete     => Some(Key::Delete),
        termion::event::Key::Left       => Some(Key::Left),
        termion::event::Key::Right      => Some(Key::Right),
        termion::event::Key::Up         => Some(Key::Up),
        termion::event::Key::Down       => Some(Key::Down),
        termion::event::Key::Char('\t') => Some(Key::Tab),
        termion::event::Key::Char('\n') => Some(Key::Enter),
        termion::event::Key::Char(ch)   => Some(Key::Char(ch)),
        termion::event::Key::Alt(ch)    => Some(Key::Alt(ch)),
        termion::event::Key::Ctrl(ch)   => Some(Key::Ctrl(ch)),
        _ => {
            trace!("unused key: {:?}", key);
            None
        }
    }
}

/// Convert escape sequences not supported by termion.
fn termion_unsupported2event(seq: &[u8]) -> Option<Event> {
    match seq {
        [27, 91, 49, 59, 53, 65] => Some(Event::Key(Key::CtrlUp)),
        [27, 91, 49, 59, 53, 66] => Some(Event::Key(Key::CtrlDown)),
        _ => {
            trace!("ignored input: {:?}", seq);
            None
        }
    }
}

fn user_input_handler(tx: Sender<Event>) {
    loop {
        for event in std::io::stdin().events() {
            let event = event.unwrap();
            match event {
                termion::event::Event::Key(key) => {
                    if let Some(key) = termion_key2input_key(key) {
                        tx.send(Event::Key(key)).ok();
                    }
                }
                termion::event::Event::Unsupported(seq) => {
                    if let Some(event) = termion_unsupported2event(&seq) {
                        tx.send(event).ok();
                    }
                }
                _ => {
                    trace!("event: {:?}", event);
                }
            }
        }
    }
}