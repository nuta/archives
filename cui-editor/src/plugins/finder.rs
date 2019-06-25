use crate::editor::{Editor, Key};
use crate::window::Point;
use crate::plugins::ActionBox;
use crate::color::{Color, color};
use termion::color;
use termion::style;
use std::fmt::Write;
use walkdir::WalkDir;

#[derive(Debug, Clone)]
enum Entry {
    File(String),
    Command((String, String)),
}

// TODO:
fn filter_by_gitignore(filepath: &str) -> bool {
    filepath.starts_with("./.git")
    || filepath.starts_with("./target")
}

fn fill_entries(editor: &Editor) -> Vec<Entry> {
    let mut entries = Vec::new();

    // Fill files.
    for entry in WalkDir::new(".") {
        if let Ok(entry) = entry {
            if entry.file_type().is_dir() {
                continue;
            }

            let filepath = entry.path().to_str().unwrap().to_owned();
            if !filter_by_gitignore(&filepath) {
                entries.push(Entry::File(filepath));
            }
        }
    }

    // Fill commands.
    for (name, command) in editor.commands().iter() {
        entries.push(Entry::Command((name.to_owned(), command.title.to_owned())));
    }

    entries
}

struct FinderActionBox {
    input: String,
    selected_index: usize,
    entries: Vec<Entry>,
    matched: Vec<Entry>,
}

impl FinderActionBox {
    fn new(entries: Vec<Entry>) -> FinderActionBox {
        let mut action_box = FinderActionBox {
            entries,
            matched: Vec::new(),
            selected_index: 0,
            input: String::new(),
        };

        action_box.search();
        action_box
    }

    fn search(&mut self) {
        let mut matched = Vec::new();
        let empty = self.input.is_empty();
        let command_only = self.input.starts_with(">");
        let query = if command_only {
            &self.input[1..]
        } else {
            &self.input
        };

        for entry in &self.entries {
            match entry {
                Entry::File(filepath) if !command_only => {
                    if empty || filepath.contains(&query) {
                        matched.push(entry.clone());
                    }
                }
                Entry::Command((_, title)) => {
                    if empty || title.contains(&query) {
                        matched.push(entry.clone());
                    }
                }
                _ => continue,
            }
        }

        self.matched = matched;
        if self.selected_index >= self.matched.len() {
            self.selected_index = self.matched.len().saturating_sub(1);
        }
    }
}

impl ActionBox for FinderActionBox {
    fn on_key_input(&mut self, editor: &mut Editor, key: &Key) {
        match key {
            Key::Char(ch) => {
                self.input.push(*ch);
            }
            Key::Enter => {
                if let Some(selected) = self.matched.get(self.selected_index) {
                    match selected {
                        Entry::File(filepath) => {
                            editor.open_file(filepath, false).ok();
                        }
                        Entry::Command((command, _)) => {
                            editor.invoke_command(command);
                        }
                    }

                    editor.current().lock().unwrap().close_action_box();
                }
            }
            Key::Backspace => {
                if !self.input.is_empty() {
                    self.input.truncate(self.input.len() - 1);
                }
            }
            Key::Up => {
                self.selected_index = self.selected_index.saturating_sub(1);
            }
            Key::Down => {
                self.selected_index += 1;
            }
            Key::Ctrl('c') => {
                editor.current().lock().unwrap().close_action_box();
            }
            _ => (),
        }

        self.search();
    }

    fn render(&self, width: usize, max_height: usize) -> (Vec<String>, Point) {
        let mut prompt_line = String::new();
        write!(
            prompt_line,
            "{}{}{}{}",
            style::Bold,
            color::Bg(color(Color::ActionBox)),
            self.input,
            " ".repeat(width - self.input.len())
        ).ok();

        let mut lines = vec![prompt_line];

        let max_results = max_height - 1 /* prompt_line */;
        let mut index = 0;
        for entry in &self.matched {
            if index >= max_results {
                break;
            }

            let title = match entry {
                Entry::Command((_, title)) => {
                    title
                }
                Entry::File(filepath) => {
                    filepath
                }
            };

            // Matched.
            let mut line = String::new();
            let padding = " ".repeat(width.saturating_sub(title.len()));
            if index == self.selected_index || index == max_results - 1 {
                write!(
                    line,
                    "{}{}{}{}{}{}",
                    style::Bold,
                    style::Underline,
                    &title[..std::cmp::min(title.len(), width)],
                    padding,
                    style::NoBold,
                    style::NoUnderline
                ).ok();
            } else {
                write!(line, "{}{}", title, padding).ok();
            }

            lines.push(line);
            index += 1;
        }

        // Render blank lines.
        for _ in 0..(max_results - index) {
            let mut line = String::new();
            write!(line, "{}", " ".repeat(width)).ok();
            lines.push(line);
        }

        let cursor_pos = Point::new(0, self.input.len());
        (lines, cursor_pos)
    }
}

fn open_finder(editor: &mut Editor) {
    let mut win = editor.current().lock().unwrap();
    let entries = fill_entries(editor);
    win.open_action_box(Box::new(FinderActionBox::new(entries)));
}

pub fn activate(editor: &mut Editor) {
    editor.register_command("finder.open", "Open Finder", open_finder);
}