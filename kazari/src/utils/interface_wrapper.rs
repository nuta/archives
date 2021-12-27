use crate::wl::{
    protocols::common::{EventSet, RequestSet},
    Connection, DeserializeError, Handle, Interface, Message, ObjectId, Opcode, RawMessage,
};
use alloc::boxed::Box;
use alloc::rc::Rc;
use core::cell::RefCell;
use core::marker::PhantomData;

pub trait InterfaceWrapper {
    fn name(&self) -> &'static str;
    fn version(&self) -> u32;
    fn as_received_request(
        &self,
        con: Rc<RefCell<dyn Connection>>,
        opcode: Opcode,
        object: ObjectId,
        message: &[u8],
        handles: &[Handle],
    ) -> Result<RequestSet, DeserializeError>;
    fn as_received_event(
        &self,
        con: Rc<RefCell<dyn Connection>>,
        opcode: Opcode,
        object: ObjectId,
        message: &[u8],
        handles: &[Handle],
    ) -> Result<EventSet, DeserializeError>;
}

struct InterfaceWrapperImpl<I: Interface> {
    _pd: PhantomData<&'static I>,
}

pub fn construct_interface_wrapper<I: Interface>() -> Box<dyn InterfaceWrapper> {
    Box::new(InterfaceWrapperImpl::<I> { _pd: PhantomData })
}

impl<I: Interface> InterfaceWrapper for InterfaceWrapperImpl<I> {
    fn name(&self) -> &'static str {
        I::NAME
    }

    fn version(&self) -> u32 {
        I::VERSION
    }

    fn as_received_request(
        &self,
        con: Rc<RefCell<dyn Connection>>,
        opcode: Opcode,
        _object: ObjectId,
        message: &[u8],
        handles: &[Handle],
    ) -> Result<RequestSet, DeserializeError> {
        let payload_types = I::PAYLOAD_TYPES[opcode.0 as usize - 1];
        trace!(
            "if=\"{}\", object={:?}, opcode={:?}, payload_types={:?}",
            self.name(),
            _object,
            opcode,
            payload_types
        );
        let raw = RawMessage::deserialize(message, payload_types, handles)?;
        I::Request::from_raw(con.clone(), &raw).map(|req| req.into_received_request())
    }

    fn as_received_event(
        &self,
        con: Rc<RefCell<dyn Connection>>,
        opcode: Opcode,
        object: ObjectId,
        message: &[u8],
        handles: &[Handle],
    ) -> Result<EventSet, DeserializeError> {
        let payload_types = I::PAYLOAD_TYPES[opcode.0 as usize - 1];
        let raw = RawMessage::deserialize(message, payload_types, handles)?;
        I::Event::from_raw(con.clone(), &raw)
            .map(|event| event.into_received_event(con.clone(), object))
    }
}
