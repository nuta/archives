use crate::utils::bytes_reader::{BytesReader, BytesReaderError};
use crate::utils::bytes_writer::{BytesWriter, BytesWriterError};
use crate::wl::connection::Connection;
use crate::wl::protocols::common::{EventSet, RequestSet};
use alloc::rc::Rc;
use alloc::str::{self, Utf8Error};
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use core::cell::RefCell;
use smallvec::SmallVec;

#[derive(Debug, Copy, Clone, PartialEq)]
pub struct Opcode(pub u16);

#[derive(Debug, Copy, Clone, PartialEq, Eq, Hash)]
pub struct ObjectId(pub u32);

impl ObjectId {
    pub fn null() -> ObjectId {
        ObjectId(0)
    }

    pub fn is_null(self) -> bool {
        self.0 == 0
    }
}

impl Into<Payload> for ObjectId {
    fn into(self) -> Payload {
        Payload::ObjectId(self)
    }
}

impl Into<Payload> for Option<String> {
    fn into(self) -> Payload {
        match self {
            Some(string) => string.into(),
            None => "".to_string().into(), // FIXME: Am I correct?
        }
    }
}

#[derive(Debug, Copy, Clone, PartialEq)]
pub struct NewId(pub u32);

impl NewId {
    pub fn as_id(&self) -> ObjectId {
        ObjectId(self.0)
    }
}

impl Into<Payload> for NewId {
    fn into(self) -> Payload {
        Payload::NewId(self)
    }
}

#[derive(Debug, Copy, Clone, PartialEq, Eq, Hash)]
pub struct Handle(pub i32);

impl Into<Payload> for Handle {
    fn into(self) -> Payload {
        Payload::Handle(self)
    }
}

#[derive(Debug, Clone, PartialEq)]
pub struct Array(pub Vec<u8>);

impl Array {
    pub fn from_bytes(bytes: &[u8]) -> Array {
        Array(Vec::from(bytes))
    }
}

impl Into<Payload> for Array {
    fn into(self) -> Payload {
        Payload::Array(self)
    }
}

impl Into<Payload> for u32 {
    fn into(self) -> Payload {
        Payload::UInt(self)
    }
}

impl Into<Payload> for i32 {
    fn into(self) -> Payload {
        Payload::Int(self)
    }
}

impl Into<Payload> for f32 {
    fn into(self) -> Payload {
        Payload::Fixed(self)
    }
}

impl Into<Payload> for String {
    fn into(self) -> Payload {
        Payload::String(self)
    }
}

#[derive(Debug, Clone, PartialEq)]
pub enum Payload {
    UInt(u32),
    Int(i32),
    Fixed(f32),
    ObjectId(ObjectId),
    NewId(NewId),
    Handle(Handle),
    Array(Array),
    String(String),
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum PayloadType {
    UInt,
    Int,
    Fixed,
    ObjectId,
    NewId,
    Handle,
    Array,
    String,
}

pub trait Message: Sized {
    fn into_raw(self, sender: ObjectId) -> RawMessage;
    fn from_raw(con: Rc<RefCell<dyn Connection>>, m: &RawMessage)
        -> Result<Self, DeserializeError>;
    fn into_received_event(self, con: Rc<RefCell<dyn Connection>>, id: ObjectId) -> EventSet;
    fn into_received_request(self) -> RequestSet;
}

#[derive(Debug, PartialEq)]
pub enum SerializeError {
    WriterError(BytesWriterError),
}

#[derive(Debug, PartialEq)]
pub enum DeserializeError {
    ReaderError(BytesReaderError),
    InvalidLength,
    TooFewHandles,
    ObjectIsNull,
    UnknownOpcode,
    UnexpectedType,
    NonTerminatedString,
    Utf8Error(Utf8Error),
}

#[derive(Debug, PartialEq)]
pub struct RawMessage {
    pub sender: ObjectId,
    pub opcode: Opcode,
    pub args: SmallVec<[Payload; 8]>,
}

impl RawMessage {
    pub fn serialize(
        &self,
        buf: &mut [u8],
        handles: &mut Vec<Handle>,
    ) -> Result<usize, SerializeError> {
        let mut writer = BytesWriter::new(buf);
        writer
            .append_le_u32(self.sender.0)
            .map_err(SerializeError::WriterError)?;
        writer
            .append_le_u32(0 /* TBD */)
            .map_err(SerializeError::WriterError)?;

        let bytes_remaining = writer.remaining_len();
        for arg in &self.args {
            match arg {
                Payload::UInt(value) => {
                    writer
                        .append_le_u32(*value)
                        .map_err(SerializeError::WriterError)?;
                }
                Payload::Int(value) => {
                    writer
                        .append_le_i32(*value)
                        .map_err(SerializeError::WriterError)?;
                }
                Payload::Fixed(_value) => {
                    unimplemented!();
                }
                Payload::ObjectId(value) => {
                    writer
                        .append_le_u32(value.0)
                        .map_err(SerializeError::WriterError)?;
                }
                Payload::NewId(value) => {
                    writer
                        .append_le_u32(value.0)
                        .map_err(SerializeError::WriterError)?;
                }
                Payload::Handle(value) => {
                    handles.push(*value);
                }
                Payload::Array(value) => {
                    writer
                        .append_le_u32(value.0.len() as u32)
                        .map_err(SerializeError::WriterError)?;
                    writer
                        .append_bytes(value.0.as_slice())
                        .map_err(SerializeError::WriterError)?;
                    writer
                        .append_until_alignment(0, 4)
                        .map_err(SerializeError::WriterError)?;
                }
                Payload::String(value) => {
                    writer
                        .append_le_u32(value.len() as u32 + 1)
                        .map_err(SerializeError::WriterError)?;
                    writer
                        .append_bytes(value.as_bytes())
                        .map_err(SerializeError::WriterError)?;
                    writer.append_u8(0).map_err(SerializeError::WriterError)?; // NUL-terminated
                    writer
                        .append_until_alignment(0, 4)
                        .map_err(SerializeError::WriterError)?;
                }
            }
        }

        // Write the correct payload length and the opcode.
        let len = 8 + bytes_remaining - writer.remaining_len();
        writer
            .write_le_u32(4, (((len as u32) << 16) | self.opcode.0 as u32))
            .map_err(SerializeError::WriterError)?;
        Ok(len)
    }

    pub fn deserialize_header(buf: &[u8]) -> Result<(ObjectId, Opcode), DeserializeError> {
        let mut reader = BytesReader::new(buf);
        let sender = ObjectId(
            reader
                .consume_le_u32()
                .map_err(DeserializeError::ReaderError)?,
        );
        let word = reader
            .consume_le_u32()
            .map_err(DeserializeError::ReaderError)?;
        let len = (word >> 16) as usize;
        let opcode = Opcode((word & 0xffff) as u16);

        if len < 8 {
            return Err(DeserializeError::InvalidLength);
        }

        Ok((sender, opcode))
    }

    pub fn deserialize(
        buf: &[u8],
        arg_types: &[PayloadType],
        handles: &[Handle],
    ) -> Result<RawMessage, DeserializeError> {
        let (sender, opcode) = Self::deserialize_header(buf)?;
        let mut reader = BytesReader::new(buf);
        reader.skip(8).map_err(DeserializeError::ReaderError)?;

        let mut args = SmallVec::new();
        let mut handles_iter = handles.iter();
        for arg_type in arg_types {
            match arg_type {
                PayloadType::UInt => {
                    args.push(Payload::UInt(
                        reader
                            .consume_le_u32()
                            .map_err(DeserializeError::ReaderError)?,
                    ));
                }
                PayloadType::Int => {
                    args.push(Payload::Int(
                        reader
                            .consume_le_i32()
                            .map_err(DeserializeError::ReaderError)?,
                    ));
                }
                PayloadType::Fixed => {
                    unimplemented!();
                }
                PayloadType::ObjectId => {
                    args.push(Payload::ObjectId(ObjectId(
                        reader
                            .consume_le_u32()
                            .map_err(DeserializeError::ReaderError)?,
                    )));
                }
                PayloadType::NewId => {
                    args.push(Payload::NewId(NewId(
                        reader
                            .consume_le_u32()
                            .map_err(DeserializeError::ReaderError)?,
                    )));
                }
                PayloadType::Handle => {
                    let handle = handles_iter
                        .next()
                        .copied()
                        .ok_or(DeserializeError::TooFewHandles)?;
                    args.push(Payload::Handle(handle));
                }
                PayloadType::Array => {
                    let array_len = reader
                        .consume_le_u32()
                        .map_err(DeserializeError::ReaderError)?
                        as usize;
                    let array = Vec::from(&reader.remaining()[..array_len]);
                    reader.skip(array_len);
                    reader
                        .skip_until_alignment(4)
                        .map_err(DeserializeError::ReaderError)?;
                    args.push(Payload::Array(Array(array)));
                }
                PayloadType::String => {
                    let str_len = reader
                        .consume_le_u32()
                        .map_err(DeserializeError::ReaderError)?
                        as usize;
                    if str_len < 1 {
                        // A string should include at least one byte for the
                        // terminating null character.
                        return Err(DeserializeError::NonTerminatedString);
                    }
                    let string = str::from_utf8(&reader.remaining()[..(str_len - 1)])
                        .map_err(DeserializeError::Utf8Error)?
                        .to_string();
                    reader.skip(str_len);
                    reader
                        .skip_until_alignment(4)
                        .map_err(DeserializeError::ReaderError)?;
                    args.push(Payload::String(string));
                }
            }
        }

        Ok(RawMessage {
            sender,
            opcode,
            args,
        })
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use smallvec::smallvec;

    #[test]
    fn message_serialization() {
        {
            let mut buf = vec![0; 12];
            let mut handles = Vec::new();
            assert_eq!(
                RawMessage {
                    sender: ObjectId(1),
                    opcode: Opcode(2),
                    args: smallvec![Payload::NewId(NewId(3))],
                }
                .serialize(&mut buf, &mut handles),
                Ok(12)
            );
            assert_eq!(buf, &[1, 0, 0, 0, 2, 0, 12, 0, 3, 0, 0, 0]);
        }
    }

    #[test]
    fn message_deserialization() {
        {
            let mut buf = &[1, 0, 0, 0, 2, 0, 12, 0, 3, 0, 0, 0];
            let types = &[PayloadType::NewId];
            let mut handles = &[];
            assert_eq!(
                RawMessage::deserialize(buf, types, handles),
                Ok(RawMessage {
                    sender: ObjectId(1),
                    opcode: Opcode(2),
                    args: smallvec![Payload::NewId(NewId(3))],
                })
            );
        }
    }
}
