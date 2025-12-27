use core::fmt;

/// A C-style string without the null terminator.
pub struct Name<'a>(&'a [u8]);

impl<'a> Name<'a> {
    pub(crate) fn new(name: &'a [u8]) -> Self {
        Self(name)
    }

    pub fn as_bytes(&self) -> &[u8] {
        self.0
    }
}

impl<'a> fmt::Debug for Name<'a> {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if let Ok(name) = core::str::from_utf8(self.0) {
            write!(f, "{}", name)
        } else {
            write!(f, "{:?}", self.0)
        }
    }
}
