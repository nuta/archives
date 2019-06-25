use crate::Result;
use std::borrow::Cow;
use std::io::Read;

// TODO: implement gap buffer or rope
// TODO: support large files
pub struct Buffer {
    filepath: String,
    text: Vec<String>,
}

impl Buffer {
    pub fn from_file(filepath: &str) -> Result<Buffer> {
        let mut file = std::fs::File::open(filepath)?;
        let mut body = String::new();
        file.read_to_string(&mut body)?;

        Ok(Buffer::from_str(filepath, &body))
    }

    pub fn from_str(filepath: &str, body: &str) -> Buffer {
        let mut text = Vec::new();
        for line in body.split("\n") {
            text.push(line.to_owned());
        }

        Buffer {
            filepath: String::from(filepath),
            text,
        }
    }

    #[inline]
    pub fn num_lines(&self) -> usize {
        self.text.len()
    }

    #[inline]
    pub fn line_len(&self, line: usize) -> usize {
        self.text.get(line).map(|l| l.len()).unwrap_or(0)
    }

    #[inline]
    pub fn filepath(&self) -> &str {
        &self.filepath
    }

    pub fn line<'a>(&'a self, y: usize, x: usize, max_width: usize) -> Option<Cow<'a, str>> {
        match self.text.get(y) {
            Some(content) => {
                if content.len() < x {
                    return None;
                }

                let width = std::cmp::min(max_width, content.len() - x);
                Some(Cow::from(&content[x..(x + width)]))
            }
            _ => None,
        }
    }

    pub fn insert(&mut self, y: usize, x: usize, ch: char) {
        match ch {
            '\n' => {
                let new_line = self.text[y][x..].to_owned();
                self.text[y].truncate(x);
                self.text.insert(y + 1, new_line);
            },
            ch => {
                self.text[y].insert(x, ch);
            }
        }
    }

    pub fn remove(&mut self, y: usize, x: usize) {
        if x == 0 {
            if y == 0 {
                // Do nothing.
                return;
            }

            let s = self.text[y].to_owned();
            self.text[y - 1] += &s;
            self.text.remove(y);
        } else {
            trace!("remove: y={}, x={} - 1", y, x);
            self.text[y].remove(x - 1);
        }
    }
}
