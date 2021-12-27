use crate::wl::{Connection, Message, NewId, ObjectId, Payload, PayloadType, RawMessage};
use alloc::rc::Rc;
use core::cell::RefCell;

pub trait Interface: 'static {
    /// The enum of event messages.
    type Event: Message;
    /// The enum of request messages.
    type Request: Message;
    /// The interface name.
    const NAME: &'static str;
    /// The interface version.
    const VERSION: u32;
    /// Payload types for each opcode.
    const PAYLOAD_TYPES: &'static [&'static [PayloadType]];

    fn new(con: Rc<RefCell<dyn Connection>>, id: ObjectId) -> Self;
    fn connection(&self) -> &Rc<RefCell<dyn Connection>>;
    fn id(&self) -> ObjectId;
    fn as_new_id(&self) -> NewId;
}

impl<I: Interface> Into<Payload> for Option<I> {
    fn into(self) -> Payload {
        match self {
            Some(object) => Payload::ObjectId(object.id()),
            None => Payload::ObjectId(ObjectId::null()),
        }
    }
}
