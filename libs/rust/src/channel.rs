use arch::{CId};

pub struct Channel {
    cid: CId
}

impl Channel {
    pub fn to_cid(&self) -> CId {
        self.cid
    }
}
