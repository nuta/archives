use crate::terminal::Terminal;
use crate::window::{Window, WindowId, CursorMove};
use crate::buffer::Buffer;
use crate::Result;
use log::trace;
use std::collections::{BTreeMap, HashMap};
use std::sync::{Arc, Mutex};
use std::sync::mpsc::{Receiver, channel};
use std::path::PathBuf;

#[derive(Debug, Copy, Clone, PartialEq, Eq, Hash)]
pub enum Key {
    Enter,
    Tab,
    Backspace,
    Delete,
    Left,
    Right,
    Up,
    Down,
    Char(char),
    Alt(char),
    Ctrl(char),
    CtrlUp,
    CtrlDown,
}

#[derive(Debug)]
pub enum Event {
    NeedsRedraw,
    Key(Key),
}

pub type CommandHandler = fn(&mut Editor);
pub struct Command {
    pub title: String,
    pub handler: CommandHandler,
}

pub struct Editor {
    running: bool,
    terminal: Terminal,
    events: Receiver<Event>,
    current: Arc<Mutex<Window>>,
    windows: BTreeMap<WindowId, Arc<Mutex<Window>>>,
    buffers: BTreeMap<String, Arc<Mutex<Buffer>>>,
    commands: HashMap<String, Command>,
    key_bindings: HashMap<Key, String>,
}

impl Editor {
    pub fn new() -> Editor {
        let window = Arc::new(Mutex::new(Window::new(1)));
        let mut windows = BTreeMap::new();
        windows.insert(1, window.clone());

        let (tx, rx) = channel();
        let terminal = Terminal::new(tx.clone());

        Editor {
            running: false,
            current: window.clone(),
            windows,
            buffers: BTreeMap::new(),
            events: rx,
            terminal,
            commands: HashMap::new(),
            key_bindings: HashMap::new(),
        }
    }

    #[inline]
    pub fn current(&self) -> &Arc<Mutex<Window>> {
        &self.current
    }

    #[inline]
    pub fn commands(&self) -> &HashMap<String, Command> {
        &self.commands
    }

    pub fn invoke_command(&mut self, id: &str) {
        if let Some(command) = self.commands.get(id) {
            (command.handler)(self);
        }
    }

    pub fn register_command(&mut self, id: &str, title: &str, handler: CommandHandler) {
        let command = Command {
            title: title.to_owned(),
            handler,
        };

        self.commands.insert(id.to_owned(), command);
    }

    pub fn register_key_binding(&mut self, key: Key, command: &str) {
        self.key_bindings.insert(key, command.to_owned());
    }

    pub fn open_file(&mut self, filepath: &str, in_new_window: bool) -> Result<()> {
        let abspath = PathBuf::from(filepath).canonicalize()?.to_str().unwrap().to_owned();
        let buffer = if let Some(buffer) = self.buffers.get(&abspath) {
            // The file is already opened.
            buffer.clone()
        } else {
            let buffer = Arc::new(Mutex::new(Buffer::from_file(filepath)?));
            self.buffers.insert(abspath, buffer.clone());
            buffer
        };

        if in_new_window {
            let window_lock = self.create_window();
            let mut window = window_lock.lock().unwrap();
            window.set_buffer(buffer);
            self.current = window_lock.clone();
        } else {
            let mut current = self.current.lock().unwrap();
            current.set_buffer(buffer);
        }

        Ok(())
    }

    fn handle_key_event(&mut self, key: Key) {
        let action_box = {
            let current = self.current().lock().unwrap();
            current.action_box().clone()
        };

        if let Some(ref action_box) = action_box {
            action_box.borrow_mut().on_key_input(self, &key);
            return;
        }

        match key {
            Key::Ctrl('c') => {
                self.current.lock().unwrap().reset_cursors();
            }
            Key::Ctrl('q') => {
                self.quit();
            }
            Key::Up => {
                self.current.lock().unwrap().move_cursors(
                    CursorMove::Relative(-1), CursorMove::Relative(0)
                );
            }
            Key::Down => {
                self.current.lock().unwrap().move_cursors(
                    CursorMove::Relative(1), CursorMove::Relative(0)
                );
            }
            Key::Left => {
                self.current.lock().unwrap().move_cursors(
                    CursorMove::Relative(0), CursorMove::Relative(-1)
                );
            }
            Key::Right => {
                self.current.lock().unwrap().move_cursors(
                    CursorMove::Relative(0), CursorMove::Relative(1)
                );
            }
            Key::Backspace => {
                self.current.lock().unwrap().remove()
            }
            Key::Tab => {
                self.current.lock().unwrap().insert('\t')
            }
            Key::Enter => {
                self.current.lock().unwrap().insert('\n')
            }
            Key::Char(ch) => {
                self.current.lock().unwrap().insert(ch)
            }
            _ => {
                if let Some(binding) = self.key_bindings.get(&key) {
                    self.invoke_command(&binding.to_owned());
                } else {
                    trace!("ignored key: {:?}", key);
                }
            }
        }
    }

    fn create_window(&mut self) -> Arc<Mutex<Window>> {
        for window_id in 1..128 {
            if !self.windows.contains_key(&window_id) {
                // The window ID is already being used.
                continue;
            }

            let window = Arc::new(Mutex::new(Window::new(window_id)));
            self.windows.insert(window_id, window.clone());
            return window;
        }

        panic!("too many windows");
    }

    fn quit(&mut self) {
        self.terminal.quit();
        self.running = false;
    }

    pub fn run(&mut self) {
        trace!("started");
        self.terminal.start();
        self.running = true;

        while self.running {
            let windows: Vec<Arc<Mutex<Window>>> = self.windows.values()
                .map(|w| w.clone()).collect();
            self.terminal.render(&self.current, &windows, false);

            let ev = self.events.recv().unwrap();
            match ev {
                Event::Key(key) => self.handle_key_event(key),
                Event::NeedsRedraw => self.terminal.force_render(&self.current, &windows),
            }
        }
    }
}
