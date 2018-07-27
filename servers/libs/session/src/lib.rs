#![no_std]
#![feature(alloc)]

#[cfg(test)] extern crate std;
#[cfg(not(test))] extern crate core as std;
extern crate alloc;
#[cfg(not(test))] extern crate resea_langitems;
extern crate resea;


use resea::{CId};
use alloc::rc::Rc;
use std::cell::RefCell;
use std::sync::atomic::{AtomicUsize, Ordering};
use alloc::BTreeMap;

pub type SessionId = usize;

pub struct Session<T> {
    #[allow(dead_code)]
    id: SessionId,
    #[allow(dead_code)]
    value: T,
}

pub struct SessionManager<T> {
    sessions: RefCell<BTreeMap<SessionId, Rc<Session<T>>>>,
    next_id: AtomicUsize,
}

impl<T> SessionManager<T> {
    pub fn new() -> SessionManager<T> {
        SessionManager {
            sessions: RefCell::new(BTreeMap::new()),
            next_id: AtomicUsize::new(1),
        }
    }

    pub fn create(&self, value: T) -> Rc<Session<T>> {
        let id = loop {
            /* Look for an unused ID. */
            let id = self.next_id.fetch_add(1, Ordering::SeqCst);
            if (*self.sessions.borrow()).get(&id).is_none() {
                break id;
            }
        };


        let sess = Rc::new(Session{ id: id, value: value });
        (*self.sessions.borrow_mut()).insert(id, sess.clone());
        sess
    }

    pub fn get(&self, id: SessionId) -> Option<Rc<Session<T>>> {
        if let Some(sess) = (*self.sessions.borrow()).get(&id) {
            Some(sess.clone())
        } else {
            None
        }
    }
}

#[cfg(test)]
mod tests {
    #[test]
    fn test() {
        assert_eq!(1, 1);
    }
}
