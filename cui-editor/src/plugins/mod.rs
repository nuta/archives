use crate::editor::{Editor, Key};
use crate::window::Point;

pub mod prelude;
pub mod finder;

pub fn load_plugins(editor: &mut Editor) {
    prelude::activate(editor);
    finder::activate(editor);
}

pub trait ActionBox {
    fn on_key_input(&mut self, editor: &mut Editor, key: &Key);
    fn render(&self, width: usize, max_height: usize) -> (Vec<String>, Point);
}
