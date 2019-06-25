//!
//!  Basic cursor movements.
//!
use crate::editor::Editor;
use crate::window::{Cursor, Point, CursorMove};

fn add_cursor_below(editor: &mut Editor) {
    let mut window = editor.current().lock().unwrap();
    let last_cursor = window.cursors().iter().last().unwrap().borrow().clone();
    match last_cursor {
        Cursor::Normal(Point { x, y }) => {
            window.add_cursor(Cursor::Normal(Point::new(y + 1, x)));
        }
    }
}

fn begin(editor: &mut Editor) {
    let mut window = editor.current().lock().unwrap();
    window.move_cursors(CursorMove::Relative(0), CursorMove::Absolute(0));
}

fn end(editor: &mut Editor) {
    let mut window = editor.current().lock().unwrap();
    window.move_cursors(CursorMove::Relative(0), CursorMove::End);
}

pub fn activate(editor: &mut Editor) {
    editor.register_command(
        "cursor.add_below",
        "add cursor below",
        add_cursor_below
    );

    editor.register_command(
        "move.begin",
        "go to the first column",
        begin
    );

    editor.register_command(
        "move.end",
        "go to the end of line",
        end
    );
}