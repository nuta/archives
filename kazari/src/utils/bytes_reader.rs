use core::convert::TryInto;

fn align_down(value: usize, align: usize) -> usize {
    value & !(align - 1)
}

fn align_up(value: usize, align: usize) -> usize {
    align_down(value + align - 1, align)
}

#[derive(Debug, PartialEq)]
pub enum BytesReaderError {
    TooShort,
}

pub struct BytesReader<'a> {
    bytes: &'a [u8],
    current: usize,
}

impl<'a> BytesReader<'a> {
    pub fn new(bytes: &[u8]) -> BytesReader<'_> {
        BytesReader { bytes, current: 0 }
    }

    pub fn remaining(&self) -> &[u8] {
        &self.bytes[self.current..]
    }

    pub fn remaining_len(&self) -> usize {
        self.bytes.len() - self.current
    }

    pub fn skip(&mut self, len: usize) -> Result<(), BytesReaderError> {
        if self.current + len > self.bytes.len() {
            return Err(BytesReaderError::TooShort);
        }

        self.current += len;
        Ok(())
    }

    pub fn skip_until_alignment(&mut self, align: usize) -> Result<(), BytesReaderError> {
        let next = align_up(self.current, align);
        if next > self.bytes.len() {
            return Err(BytesReaderError::TooShort);
        }

        self.current = next;
        Ok(())
    }

    pub fn consume_le_u16(&mut self) -> Result<u16, BytesReaderError> {
        if self.remaining_len() < 2 {
            return Err(BytesReaderError::TooShort);
        }

        let value = u16::from_le_bytes(
            self.bytes[self.current..self.current + 2]
                .try_into()
                .unwrap(),
        );
        self.current += 2;
        Ok(value)
    }

    pub fn consume_le_u32(&mut self) -> Result<u32, BytesReaderError> {
        if self.remaining_len() < 4 {
            return Err(BytesReaderError::TooShort);
        }

        let value = u32::from_le_bytes(
            self.bytes[self.current..self.current + 4]
                .try_into()
                .unwrap(),
        );
        self.current += 4;
        Ok(value)
    }

    pub fn consume_le_i32(&mut self) -> Result<i32, BytesReaderError> {
        if self.remaining_len() < 4 {
            return Err(BytesReaderError::TooShort);
        }

        let value = i32::from_le_bytes(
            self.bytes[self.current..self.current + 4]
                .try_into()
                .unwrap(),
        );
        self.current += 4;
        Ok(value)
    }
}
