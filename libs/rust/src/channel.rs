use arch::{CId};

pub struct Channel {
    cid: CId
}

impl Channel {
    pub fn from_cid(cid: CId) -> Channel {
        Channel { cid: cid }
    }

    pub fn to_cid(&self) -> CId {
        self.cid
    }
}
