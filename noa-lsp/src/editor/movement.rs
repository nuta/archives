use std::{cmp::max, collections::HashMap};

use noa_buffer::{
    buffer::Buffer,
    cursor::{Cursor, CursorId, Position, Range},
};

use crate::view::View;

pub struct MovementState {
    visual_xs: HashMap<CursorId, usize>,
}

impl MovementState {
    pub fn new() -> MovementState {
        MovementState {
            visual_xs: HashMap::new(),
        }
    }

    pub fn movement<'a>(&'a mut self, buffer: &'a mut Buffer, view: &'a mut View) -> Movement<'a> {
        Movement {
            state: self,
            buffer,
            view,
        }
    }
}

pub struct Movement<'a> {
    state: &'a mut MovementState,
    buffer: &'a mut Buffer,
    view: &'a mut View,
}

impl<'a> Movement<'a> {
    /// Moves the cursor to left by one grapheme.
    pub fn move_cursors_left(&mut self) {
        self.update_cursors_with(|buffer, _, _, c| c.move_left(buffer));
        self.state.visual_xs.clear();
    }

    /// Moves the cursor to right by one grapheme.
    pub fn move_cursors_right(&mut self) {
        self.update_cursors_with(|buffer, _, _, c| c.move_right(buffer));
        self.state.visual_xs.clear();
    }

    /// Moves the cursor to up by one display row (respecting soft wrapping).
    pub fn move_cursors_up(&mut self) {
        self.move_cursors_vertically(-1, |c, pos, _| c.move_to_pos(pos));
    }

    /// Moves the cursor to down by one display row (respecting soft wrapping).
    pub fn move_cursors_down(&mut self) {
        self.move_cursors_vertically(1, |c, pos, _| c.move_to_pos(pos));
    }

    pub fn scroll_up(&mut self) {
        debug_assert!(self
            .view
            .visible_range()
            .contains_or_contacts_with(self.buffer.main_cursor().moving_position()));

        for _ in 0..5 {
            self.view.scroll_up();
        }

        let visible_range = self.view.visible_range();
        while !visible_range.contains_or_contacts_with(self.buffer.main_cursor().moving_position())
        {
            self.move_cursors_up();
        }
    }

    pub fn scroll_down(&mut self) {
        debug_assert!(self
            .view
            .visible_range()
            .contains_or_contacts_with(self.buffer.main_cursor().moving_position()));

        for _ in 0..5 {
            self.view.scroll_down();
        }

        let visible_range = self.view.visible_range();
        while !visible_range.contains_or_contacts_with(self.buffer.main_cursor().moving_position())
        {
            self.move_cursors_down();
        }
    }

    pub fn add_cursors_up(&mut self) {
        self.add_cursors_vertically(-1);
    }

    pub fn add_cursors_down(&mut self) {
        self.add_cursors_vertically(1);
    }

    pub fn add_cursors_vertically(&mut self, y_diff: isize) {
        let mut new_selections = Vec::new();
        for c in self.buffer.cursors() {
            let s = c.selection();
            let n = if s.front().y == s.back().y {
                s.back().x - s.front().x
            } else {
                0
            };

            let x = self
                .state
                .visual_xs
                .get(&c.id())
                .cloned()
                .unwrap_or(s.front().x);
            let y = if y_diff < 0 {
                s.front().y.saturating_sub(y_diff.abs() as usize)
            } else {
                s.back().y + y_diff.abs() as usize
            };

            let range = self.buffer.clamp_range(Range::new(y, x, y, x + n));
            new_selections.push((range, x));
        }

        for (selection, visual_x) in new_selections {
            // Note: the newly added cursor could be deleted due to overlapping.
            let new_cursor_id = self.buffer.add_cursor(selection);
            self.state.visual_xs.insert(new_cursor_id, visual_x);
        }
    }

    pub fn select_up(&mut self) {
        self.move_cursors_vertically(-1, |c, pos, _| {
            c.move_moving_position_to(pos);
        });
    }
    pub fn select_down(&mut self) {
        self.move_cursors_vertically(1, |c, pos, _| {
            c.move_moving_position_to(pos);
        });
    }
    pub fn select_left(&mut self) {
        self.move_cursors_horizontally(-1, |c, pos| {
            c.move_moving_position_to(pos);
        });
    }
    pub fn select_right(&mut self) {
        self.move_cursors_horizontally(1, |c, pos| {
            c.move_moving_position_to(pos);
        });
    }

    pub fn select_until_beginning_of_line(&mut self) {
        self.buffer.deselect_cursors();

        self.update_cursors_with(|buffer, _, _, c| {
            let pos = c.moving_position();
            let left = pos.x;

            let mut new_pos = c.moving_position();
            new_pos.move_by(buffer, 0, 0, left, 0);
            c.move_moving_position_to(new_pos);
        });
    }

    pub fn select_until_end_of_line(&mut self) {
        self.buffer.deselect_cursors();

        self.update_cursors_with(|buffer, _, _, c| {
            let pos = c.moving_position();
            let right = buffer.line_len(pos.y) - pos.x;

            let mut new_pos = c.moving_position();
            new_pos.move_by(buffer, 0, 0, 0, right);
            c.move_moving_position_to(new_pos);
        });
    }

    fn move_cursors_vertically<F>(&mut self, y_diff: isize, mut f: F)
    where
        F: FnMut(&mut Cursor, Position, usize),
    {
        let visual_xs = self.state.visual_xs.clone();
        let mut new_visual_xs = HashMap::new();
        self.update_cursors_with(|_buffer, view, _state, c| {
            let (i_y, i_x) = match view.locate_row_by_position(c.moving_position()) {
                Some(yx) => yx,
                None => {
                    warn!(
                        "move_cursors_vertically: locate_row_by_position failed: {:?}",
                        c.moving_position()
                    );
                    return;
                }
            };

            let dest_row = view.all_rows().get(if y_diff > 0 {
                i_y.saturating_add(y_diff.abs() as usize)
            } else {
                i_y.saturating_sub(y_diff.abs() as usize)
            });

            if let Some(dest_row) = dest_row {
                let visual_x = visual_xs.get(&c.id()).copied();
                let new_visual_x = visual_x.unwrap_or(i_x);
                let new_pos = dest_row
                    .positions
                    .get(max(i_x, new_visual_x))
                    .copied()
                    .unwrap_or_else(|| dest_row.end_of_row_position());
                f(c, new_pos, new_visual_x);
                new_visual_xs.insert(c.id(), new_visual_x);
            }
        });
        self.state.visual_xs = new_visual_xs;
    }

    fn move_cursors_horizontally<F>(&mut self, x_diff: isize, f: F)
    where
        F: Fn(&mut Cursor, Position),
    {
        let (left, right) = if x_diff > 0 {
            (0, x_diff.abs() as usize)
        } else {
            (x_diff.abs() as usize, 0)
        };

        self.update_cursors_with(|buffer, _, _, c| {
            let mut new_pos = c.moving_position();
            new_pos.move_by(buffer, 0, 0, left, right);
            f(c, new_pos);
        });
    }

    // TODO: Use Buffer::updater_cursors_with() once the borrow checker supports
    //       using &mut self.view in its closure.
    fn update_cursors_with<F>(&mut self, mut f: F)
    where
        F: FnMut(&Buffer, &View, &MovementState, &mut Cursor),
    {
        let mut new_cursors = self.buffer.cursors().to_vec();
        for c in &mut new_cursors {
            f(self.buffer, self.view, self.state, c);
        }
        self.buffer.update_cursors(&new_cursors);
    }
}

#[cfg(test)]
mod tests {
    use pretty_assertions::assert_eq;

    use super::*;

    #[test]
    fn cursor_movement() {
        // ABC
        // 12
        // XYZ
        let mut buffer = Buffer::from_text("ABC\n12\nXYZ");
        let mut view = View::new();
        view.layout(&buffer, 16, 5);
        let mut movement_state = MovementState::new();
        let mut movement = movement_state.movement(&mut buffer, &mut view);

        movement.buffer.set_cursors_for_test(&[Cursor::new(2, 1)]);
        movement.move_cursors_up();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(1, 1)]);
        movement.move_cursors_up();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(0, 1)]);
        movement.move_cursors_up();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(0, 1)]);

        movement.buffer.set_cursors_for_test(&[Cursor::new(0, 1)]);
        movement.move_cursors_down();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(1, 1)]);
        movement.move_cursors_down();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(2, 1)]);
        movement.move_cursors_down();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(2, 1)]);
    }

    #[test]
    fn cursor_movement_softwrapped() {
        // ABCDE
        // XY
        //
        let mut buffer = Buffer::from_text("ABCDEXY\n");
        let mut view = View::new();
        view.layout(&buffer, 3, 5);
        let mut movement_state = MovementState::new();
        let mut movement = movement_state.movement(&mut buffer, &mut view);

        movement.buffer.set_cursors_for_test(&[Cursor::new(0, 7)]);
        movement.move_cursors_up();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(0, 2)]);
        movement.move_cursors_up();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(0, 2)]);

        movement.buffer.set_cursors_for_test(&[Cursor::new(1, 0)]);
        movement.move_cursors_up();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(0, 7)]);
        movement.move_cursors_up();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(0, 2)]);

        movement.buffer.set_cursors_for_test(&[Cursor::new(0, 4)]);
        movement.move_cursors_down();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(0, 7)]);
        movement.move_cursors_down();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(1, 0)]);
    }

    #[test]
    fn cursor_movement_at_end_of_line() {
        // ABC
        // ABC
        // ABC
        let mut buffer = Buffer::from_text("ABC\nABC\nABC");
        let mut view = View::new();
        view.layout(&buffer, 16, 5);
        let mut movement_state = MovementState::new();
        let mut movement = movement_state.movement(&mut buffer, &mut view);

        movement.buffer.set_cursors_for_test(&[Cursor::new(2, 3)]);
        movement.move_cursors_up();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(1, 3)]);
        movement.move_cursors_up();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(0, 3)]);
        movement.move_cursors_up();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(0, 3)]);

        movement.buffer.set_cursors_for_test(&[Cursor::new(0, 3)]);
        movement.move_cursors_down();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(1, 3)]);
        movement.move_cursors_down();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(2, 3)]);
        movement.move_cursors_down();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(2, 3)]);
    }

    #[test]
    fn cursor_movement_through_empty_text() {
        let mut buffer = Buffer::from_text("");
        let mut view = View::new();
        view.layout(&buffer, 16, 5);
        let mut movement_state = MovementState::new();
        let mut movement = movement_state.movement(&mut buffer, &mut view);

        movement.buffer.set_cursors_for_test(&[Cursor::new(0, 0)]);
        movement.move_cursors_up();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(0, 0)]);
        movement.move_cursors_down();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(0, 0)]);
    }

    #[test]
    fn cursor_movement_through_empty_lines() {
        // ""
        // ""
        // ""
        let mut buffer = Buffer::from_text("\n\n");
        let mut view = View::new();
        view.layout(&buffer, 16, 5);
        let mut movement_state = MovementState::new();
        let mut movement = movement_state.movement(&mut buffer, &mut view);

        movement.buffer.set_cursors_for_test(&[Cursor::new(2, 0)]);
        movement.move_cursors_up();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(1, 0)]);
        movement.move_cursors_up();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(0, 0)]);
        movement.move_cursors_up();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(0, 0)]);

        movement.buffer.set_cursors_for_test(&[Cursor::new(0, 0)]);
        movement.move_cursors_down();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(1, 0)]);
        movement.move_cursors_down();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(2, 0)]);
        movement.move_cursors_down();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(2, 0)]);
    }

    #[test]
    fn cursor_movement_preserving_visual_x() {
        // "ABCDEFG"
        // "123"
        // ""
        // "HIJKLMN"
        let mut buffer = Buffer::from_text("ABCDEFG\n123\n\nHIJKLMN");
        let mut view = View::new();
        view.layout(&buffer, 16, 10);
        let mut movement_state = MovementState::new();
        let mut movement = movement_state.movement(&mut buffer, &mut view);

        movement.buffer.set_cursors_for_test(&[Cursor::new(3, 5)]);
        movement.move_cursors_up();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(2, 0)]);
        movement.move_cursors_up();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(1, 3)]);
        movement.move_cursors_up();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(0, 5)]);
        movement.move_cursors_up();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(0, 5)]);

        movement.buffer.set_cursors_for_test(&[Cursor::new(0, 5)]);
        movement.move_cursors_down();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(1, 3)]);
        movement.move_cursors_down();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(2, 0)]);
        movement.move_cursors_down();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(3, 5)]);
        movement.move_cursors_down();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(3, 5)]);
    }

    #[test]
    fn select_horizontally() {
        let mut buffer = Buffer::from_text("ABC");
        let mut view = View::new();
        view.layout(&buffer, 16, 10);
        let mut movement_state = MovementState::new();
        let mut movement = movement_state.movement(&mut buffer, &mut view);

        movement.buffer.set_cursors_for_test(&[Cursor::new(0, 0)]);
        movement.select_left();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(0, 0)]);
        movement.buffer.set_cursors_for_test(&[Cursor::new(0, 1)]);
        movement.select_left();
        assert_eq!(
            movement.buffer.cursors(),
            &[Cursor::new_selection(0, 1, 0, 0)]
        );
    }

    #[test]
    fn add_cursors_up() {
        // ""
        let mut buffer = Buffer::from_text("");
        let mut view = View::new();
        view.layout(&buffer, 25, 80);
        let mut movement_state = MovementState::new();
        let mut movement = movement_state.movement(&mut buffer, &mut view);

        movement.buffer.set_cursors_for_test(&[Cursor::new(0, 0)]);
        movement.add_cursors_up();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(0, 0),]);

        // xxxxx
        // xxx
        // xxxxx
        let mut buffer = Buffer::from_text("xxxxxx\nxxx\nxxxxx");
        let mut view = View::new();
        view.layout(&buffer, 25, 80);
        let mut movement_state = MovementState::new();
        let mut movement = movement_state.movement(&mut buffer, &mut view);

        movement
            .buffer
            .set_cursors_for_test(&[Cursor::new_selection(2, 0, 2, 2)]);
        movement.add_cursors_up();
        assert_eq!(
            movement.buffer.cursors(),
            &[
                Cursor::new_selection(1, 0, 1, 2),
                Cursor::new_selection(2, 0, 2, 2)
            ]
        );
        movement.add_cursors_up();
        assert_eq!(
            movement.buffer.cursors(),
            &[
                Cursor::new_selection(0, 0, 0, 2),
                Cursor::new_selection(1, 0, 1, 2),
                Cursor::new_selection(2, 0, 2, 2)
            ]
        );
    }

    #[test]
    fn add_cursors_up_preserving_visual_x() {
        // xxxxx
        // xxx
        // xxxxx
        let mut buffer = Buffer::from_text("xxxxxx\nxxx\nxxxxx");
        let mut view = View::new();
        view.layout(&buffer, 25, 80);
        let mut movement_state = MovementState::new();
        let mut movement = movement_state.movement(&mut buffer, &mut view);

        movement.buffer.set_cursors_for_test(&[Cursor::new(2, 4)]);
        movement.add_cursors_up();
        assert_eq!(
            movement.buffer.cursors(),
            &[Cursor::new(1, 3), Cursor::new(2, 4)]
        );
        movement.add_cursors_up();
        assert_eq!(
            movement.buffer.cursors(),
            &[Cursor::new(0, 4), Cursor::new(1, 3), Cursor::new(2, 4)]
        );
    }

    #[test]
    fn add_cursors_down() {
        // ""
        let mut buffer = Buffer::from_text("");
        let mut view = View::new();
        view.layout(&buffer, 25, 80);
        let mut movement_state = MovementState::new();
        let mut movement = movement_state.movement(&mut buffer, &mut view);

        movement.buffer.set_cursors_for_test(&[Cursor::new(0, 0)]);
        movement.add_cursors_down();
        assert_eq!(movement.buffer.cursors(), &[Cursor::new(0, 0),]);

        // xxxxx
        // xxx
        // xxxxx
        let mut buffer = Buffer::from_text("xxxxxx\nxxx\nxxxxx");
        let mut view = View::new();
        view.layout(&buffer, 25, 80);
        let mut movement_state = MovementState::new();
        let mut movement = movement_state.movement(&mut buffer, &mut view);

        movement
            .buffer
            .set_cursors_for_test(&[Cursor::new_selection(0, 0, 0, 2)]);
        movement.add_cursors_down();
        assert_eq!(
            movement.buffer.cursors(),
            &[
                Cursor::new_selection(0, 0, 0, 2),
                Cursor::new_selection(1, 0, 1, 2),
            ]
        );
        movement.add_cursors_down();
        assert_eq!(
            movement.buffer.cursors(),
            &[
                Cursor::new_selection(0, 0, 0, 2),
                Cursor::new_selection(1, 0, 1, 2),
                Cursor::new_selection(2, 0, 2, 2)
            ]
        );
    }

    #[test]
    fn add_cursors_down_preserving_visual_x() {
        // xxxxx
        // xxx
        // xxxxx
        let mut buffer = Buffer::from_text("xxxxxx\nxxx\nxxxxx");
        let mut view = View::new();
        view.layout(&buffer, 25, 80);
        let mut movement_state = MovementState::new();
        let mut movement = movement_state.movement(&mut buffer, &mut view);

        movement.buffer.set_cursors_for_test(&[Cursor::new(0, 4)]);
        movement.add_cursors_down();
        assert_eq!(
            movement.buffer.cursors(),
            &[Cursor::new(0, 4), Cursor::new(1, 3)]
        );
        movement.add_cursors_down();
        assert_eq!(
            movement.buffer.cursors(),
            &[Cursor::new(0, 4), Cursor::new(1, 3), Cursor::new(2, 4)]
        );
    }
}
