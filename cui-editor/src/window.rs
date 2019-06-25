use crate::buffer::Buffer;
use crate::plugins::ActionBox;
use std::cell::RefCell;
use std::sync::{Arc, Mutex};
use std::fmt;
use lazy_static::lazy_static;

#[derive(Copy, Clone, PartialEq, Eq)]
pub struct Point {
    pub x: usize,
    pub y: usize,
}

impl Point {
    #[inline(always)]
    pub fn new(y: usize, x: usize) -> Point {
        Point { x, y }
    }
}

impl fmt::Display for Point {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "({}, {})", self.x, self.y)
    }
}

impl fmt::Debug for Point {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "({}, {})", self.x, self.y)
    }
}

#[derive(Debug, Copy, Clone)]
pub enum CursorMove {
    Relative(isize),
    Absolute(usize),
    End,
}

#[derive(Debug, Copy, Clone, PartialEq, Eq)]
pub enum Cursor {
    Normal(Point),
}

pub type WindowId = usize;

///  ```
///   ____________Buffer (file)____________
///  |                  |                  |
///  |     win.offset   |                  |
///  |        *-------------------+        |
///  |        |         |         |        |
///  |        |         |         |        |
///  |        |         |         |        |
///  |________|_________*         |        |
///  |        |       cursor      |        |
///  |        +-------------------+        |
///  |                                     |
///  |                                     |
///   --------------------------------------
///  ```
///
pub struct Window {
    id: WindowId,
    buffer: Arc<Mutex<Buffer>>,
    offset: Point,
    cursors: Vec<RefCell<Cursor>>,
    needs_redraw: bool,
    action_box: Option<Arc<RefCell<Box<ActionBox>>>>,
}

lazy_static! {
    static ref SCRATCH: Arc<Mutex<Buffer>> = {
        Arc::new(Mutex::new(Buffer::from_str("*scratch*", "")))
    };
}

impl Window {
    pub fn new(id: usize) -> Window {
        let win = Window {
            id,
            needs_redraw: true,
            buffer: SCRATCH.clone(),
            offset: Point::new(0, 0),
            cursors: vec![RefCell::new(Cursor::Normal(Point::new(0, 0)))],
            action_box: None,
        };

        win
    }

    #[inline]
    pub fn id(&self) -> usize {
        self.id
    }

    #[inline]
    pub fn buffer(&self) -> &Arc<Mutex<Buffer>> {
        &self.buffer
    }

    #[inline]
    pub fn cursors(&self) -> &[RefCell<Cursor>] {
        &self.cursors
    }

    #[inline]
    pub fn reset_cursors(&mut self) {
        if self.cursors.len() > 1 {
            self.cursors = vec![self.cursors[0].clone()];
            self.needs_redraw = true;
        }
    }

    #[inline]
    pub fn offset(&self) -> &Point {
        &self.offset
    }

    #[inline]
    pub fn needs_redraw(&self) -> bool {
        self.needs_redraw
    }

    #[inline]
    pub fn reset_needs_redraw(&mut self) {
        self.needs_redraw = false;
    }

    #[inline]
    pub fn action_box(&self) -> &Option<Arc<RefCell<Box<ActionBox>>>> {
        &self.action_box
    }

    #[inline]
    pub fn open_action_box(&mut self, action_box: Box<ActionBox>) {
        self.reset_cursors();
        self.action_box = Some(Arc::new(RefCell::new(action_box)));
        self.needs_redraw = true;
    }

    #[inline]
    pub fn close_action_box(&mut self) {
        self.reset_cursors();
        self.action_box = None;
        self.needs_redraw = true;
    }

    pub fn add_cursor(&mut self, cursor: Cursor) {
        self.cursors.push(RefCell::new(cursor));
    }

    /// Moves the offset if the cursor is out of the window.
    pub fn update_offset(&mut self, width: usize, height: usize) {
        match *self.cursors[0].borrow() {
            Cursor::Normal(Point { x: cur_x, y: cur_y }) => {
                let new_x = if cur_x >= self.offset.x + width {
                    cur_x - width + 1
                } else if cur_x < self.offset.x {
                    cur_x
                } else {
                    self.offset.x
                };

                let new_y = if cur_y >= self.offset.y + height {
                    cur_y - height + 1
                } else if cur_y < self.offset.y {
                    cur_y
                } else {
                    self.offset.y
                };

                let new_offset = Point::new(new_y, new_x);
                if new_offset != self.offset {
                    self.needs_redraw = true;
                    self.offset = new_offset;
                }
            }
        }
    }

    pub fn set_buffer(&mut self, buffer: Arc<Mutex<Buffer>>) {
        self.reset_cursors();
        self.buffer = buffer;
        self.needs_redraw = true;
    }

    pub fn move_cursors(&mut self, y_offset: CursorMove, x_offset: CursorMove) {
        for i in 0..self.cursors.len() {
            self.move_cursor(i, y_offset, x_offset);
        }
    }

    pub fn move_cursor(&self, index: usize, y_offset: CursorMove, x_offset: CursorMove) {
        let new_cursor = match *self.cursors[index].borrow() {
            Cursor::Normal(Point { x: old_x, y: old_y }) => {
                let buffer = self.buffer.lock().unwrap();
                let num_lines = buffer.num_lines();

                let overflowable_new_y = match y_offset {
                    CursorMove::Relative(offset) if offset > 0 => {
                        old_y.saturating_add(offset as usize)
                    }
                    CursorMove::Relative(offset) => {
                        old_y.saturating_sub(offset.abs() as usize)
                    }
                    CursorMove::Absolute(value) => {
                        value
                    },
                    CursorMove::End => {
                        num_lines
                    }
                };
                let mut new_y = std::cmp::min(overflowable_new_y, num_lines - 1);

                let overflowable_new_x = match x_offset {
                    CursorMove::Relative(offset) if offset > 0 => {
                        old_x.saturating_add(offset as usize)
                    }
                    CursorMove::Relative(offset) => {
                        if let Some(new) = old_x.checked_sub(offset.abs() as usize) {
                            new
                        } else if new_y > 0 {
                            let prev_line_len = {
                                let prev_line_len = buffer.line_len(new_y.saturating_sub(1));
                                prev_line_len
                            };

                            new_y -= 1;
                            prev_line_len
                        } else {
                            old_x
                        }
                    }
                    CursorMove::Absolute(value) => {
                        value
                    }
                    CursorMove::End => {
                        buffer.line_len(new_y)
                    }
                };
                let new_x = std::cmp::min(overflowable_new_x, buffer.line_len(new_y));

                Cursor::Normal(Point::new(new_y, new_x))
            }
        };

        self.cursors[index].replace(new_cursor);
    }

    pub fn merge_cursors(&mut self) {
        // Merge cursors at same position.
        let mut new_cursors = Vec::new();
        for cursor in &self.cursors {
            let cursor = cursor.borrow().clone();
            if new_cursors.iter().any(|c: &RefCell<Cursor>| *c.borrow() == cursor) {
                continue;
            }

            new_cursors.push(RefCell::new(cursor));
        }

        self.cursors = new_cursors;
    }

    pub fn insert(&mut self, ch: char) {
        self.needs_redraw = true;

        let num_cursors = self.cursors().len();
        for (index, cursor) in self.cursors.iter().enumerate() {
            let cursor = cursor.borrow().clone();
            match cursor {
                Cursor::Normal(Point { x: cur_x, y: cur_y }) => {
                    self.buffer.lock().unwrap().insert(cur_y, cur_x, ch);

                    if ch == '\n' {
                        for i in index..num_cursors {
                            let cursor_i = self.cursors[i].borrow().clone();
                            match cursor_i {
                                Cursor::Normal(Point { y, x }) if y == cur_y && i != index => {
                                    let x_offset = x.saturating_sub(cur_x);
                                    self.move_cursor(i, CursorMove::Relative(1), CursorMove::Absolute(x_offset));
                                }
                                Cursor::Normal(_) => {
                                    debug!("move below: {}", i);
                                    self.move_cursor(i, CursorMove::Relative(1), CursorMove::Relative(0));
                                }
                            }
                        }
                    } else {
                        for i in index..num_cursors {
                            let cursor_i = self.cursors[i].borrow().clone();
                            match cursor_i {
                                Cursor::Normal(Point { y, .. }) if y == cur_y => {
                                    self.move_cursor(i, CursorMove::Relative(0), CursorMove::Relative(1));
                                }
                                _ => (),
                            }
                        }
                    }
                }
            }
        }

        if ch == '\n' {
            self.move_cursors(CursorMove::Relative(0), CursorMove::Absolute(0));
        }

        self.merge_cursors();
    }

    pub fn remove(&mut self) {
        self.needs_redraw = true;

        let num_cursors = self.cursors().len();
        for (index, cursor) in self.cursors.iter().enumerate() {
            let cursor = cursor.borrow().clone();
            match cursor {
                Cursor::Normal(Point { x: cur_x, y: cur_y }) => {
                    self.move_cursor(index, CursorMove::Relative(0), CursorMove::Relative(-1));

                    let mut buffer = self.buffer.lock().unwrap();
                    let x = std::cmp::min(cur_x, buffer.line_len(cur_y));
                    buffer.remove(cur_y, x);
                    drop(buffer);

                    if cur_x == 0 {
                        for i in (index + 1)..num_cursors {
                            debug!("remove to above {}", i);
                            self.move_cursor(i, CursorMove::Relative(-1), CursorMove::Relative(0));
                        }
                    } else {
                        for i in (index + 1)..num_cursors {
                            self.move_cursor(i, CursorMove::Relative(0), CursorMove::Relative(-1));
                        }
                    }
                }
            }
        }

        self.merge_cursors();
    }
}