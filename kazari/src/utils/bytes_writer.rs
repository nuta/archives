use core::mem::size_of_val;

fn align_down(value: usize, align: usize) -> usize {
    value & !(align - 1)
}

fn align_up(value: usize, align: usize) -> usize {
    align_down(value + align - 1, align)
}

#[derive(Debug, PartialEq)]
pub enum BytesWriterError {
    TooShort,
    OutOfBounds,
}

pub struct BytesWriter<'a> {
    bytes: &'a mut [u8],
    current: usize,
}

impl<'a> BytesWriter<'a> {
    pub fn new(bytes: &mut [u8]) -> BytesWriter<'_> {
        BytesWriter { bytes, current: 0 }
    }

    pub fn written_len(&self) -> usize {
        self.current
    }

    pub fn remaining_len(&self) -> usize {
        self.bytes.len() - self.current
    }

    pub fn append_until_alignment(
        &mut self,
        value: u8,
        align: usize,
    ) -> Result<(), BytesWriterError> {
        let next = align_up(self.current, align);
        if next > self.bytes.len() {
            return Err(BytesWriterError::TooShort);
        }

        self.bytes[self.current..next].fill(value);
        self.current = next;
        Ok(())
    }

    pub fn append_bytes(&mut self, value: &[u8]) -> Result<(), BytesWriterError> {
        if self.remaining_len() < value.len() {
            return Err(BytesWriterError::TooShort);
        }

        self.bytes[self.current..self.current + value.len()].copy_from_slice(value);
        self.current += value.len();
        Ok(())
    }

    pub fn append_u8(&mut self, value: u8) -> Result<(), BytesWriterError> {
        if self.remaining_len() < size_of_val(&value) {
            return Err(BytesWriterError::TooShort);
        }

        self.bytes[self.current] = value;
        self.current += 1;
        Ok(())
    }

    pub fn append_le_u16(&mut self, value: u16) -> Result<(), BytesWriterError> {
        if self.remaining_len() < size_of_val(&value) {
            return Err(BytesWriterError::TooShort);
        }

        self.write_le_u16(self.current, value)?;
        self.current += size_of_val(&value);
        Ok(())
    }

    pub fn append_le_u32(&mut self, value: u32) -> Result<(), BytesWriterError> {
        if self.remaining_len() < size_of_val(&value) {
            return Err(BytesWriterError::TooShort);
        }

        self.write_le_u32(self.current, value)?;
        self.current += size_of_val(&value);
        Ok(())
    }

    pub fn append_le_i32(&mut self, value: i32) -> Result<(), BytesWriterError> {
        if self.remaining_len() < size_of_val(&value) {
            return Err(BytesWriterError::TooShort);
        }

        let data = i32::to_le_bytes(value);
        self.bytes[self.current..self.current + data.len()].copy_from_slice(&data[..]);
        self.current += data.len();
        Ok(())
    }

    pub fn write_le_u16(&mut self, offset: usize, value: u16) -> Result<(), BytesWriterError> {
        if offset + size_of_val(&value) > self.bytes.len() {
            return Err(BytesWriterError::OutOfBounds);
        }

        let data = u16::to_le_bytes(value);
        self.bytes[offset..offset + data.len()].copy_from_slice(&data[..]);
        Ok(())
    }

    pub fn write_le_u32(&mut self, offset: usize, value: u32) -> Result<(), BytesWriterError> {
        if offset + size_of_val(&value) > self.bytes.len() {
            return Err(BytesWriterError::OutOfBounds);
        }

        let data = u32::to_le_bytes(value);
        self.bytes[offset..offset + data.len()].copy_from_slice(&data[..]);
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn append_uint() {
        let mut buf = vec![0; 9];
        let mut writer = BytesWriter::new(&mut buf);
        assert_eq!(writer.append_le_u32(0x12345678), Ok(()));
        assert_eq!(writer.remaining_len(), 5);
        assert_eq!(writer.append_le_u16(0xaabb), Ok(()));
        assert_eq!(writer.remaining_len(), 3);
        assert_eq!(writer.append_bytes(&[0x7, 0x8]), Ok(()));
        assert_eq!(writer.remaining_len(), 1);
        assert_eq!(writer.append_u8(0x9), Ok(()));
        assert_eq!(&buf, &[0x78, 0x56, 0x34, 0x12, 0xbb, 0xaa, 0x7, 0x8, 0x9]);
    }
}
