//! An intrusive linked list.
//!
//! ```ignore
//! use intrusive::{define_list_node, linked_list::{List, ListLink}};
//! use std::sync::Arc;
//!
//! struct Thread {
//!     id: usize,
//!     link: ListLink<ThreadsNode>,
//! }
//!
//! define_list_node!(ThreadsNode, Arc<Thread>, link);
//!
//! let mut threads = List::<ThreadsNode>::new();
//! let thread1 = Arc::new(Thread { id: 1, link: Default::default() });
//! threads.push_back(thread1);
//! ```
//!
use core::cell::Cell;
use core::fmt::{self, Debug, Formatter};
use core::marker::PhantomData;
use core::ops::ControlFlow;
use core::ptr::NonNull;
use core::sync::atomic::{AtomicBool, Ordering};

pub use etc::offset_of;

use crate::Static;

/// A trait represents a container that can be inserted into the linked list.
pub trait ListNode {
    type Elem: Static;
    fn elem_to_link(elem: Self::Elem) -> NonNull<ListLink<Self>>;
    fn from_link_to_elem(link: NonNull<ListLink<Self>>) -> Self::Elem;
    fn from_link_to_nonnull(
        link: NonNull<ListLink<Self>>,
    ) -> NonNull<<Self::Elem as Static>::Inner>;
}

/// A link fields of the linked list embedded in a container.
pub struct ListLink<L: ?Sized> {
    push_lock: AtomicBool,
    next: Cell<Option<NonNull<ListLink<L>>>>,
    prev: Cell<Option<NonNull<ListLink<L>>>>,
    _pd: PhantomData<L>,
}

impl<L: ListNode> ListLink<L> {
    pub fn empty() -> ListLink<L> {
        ListLink {
            push_lock: AtomicBool::new(true),
            next: Cell::new(None),
            prev: Cell::new(None),
            _pd: PhantomData,
        }
    }

    pub fn is_in_use(&self) -> bool {
        match (self.next.get(), self.prev.get()) {
            (Some(_), Some(_)) => true,
            (None, None) => false,
            _ => unreachable!(),
        }
    }

    fn next(&self) -> Option<NonNull<ListLink<L>>> {
        self.next.get()
    }
}

impl<L: ListNode> Debug for ListLink<L> {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "ListLink<{}>",
            if self.is_in_use() {
                "in list"
            } else {
                "not in list"
            }
        )
    }
}

impl<L: ListNode> Default for ListLink<L> {
    fn default() -> Self {
        ListLink::empty()
    }
}

// SAFETY: ListLink is protected by `push_lock`.
unsafe impl<L: ListNode + Sync> Sync for ListLink<L> {}

#[macro_export(local_inner_macros)]
macro_rules! define_list_node {
    ($list_name:ident, $elem:ty, $field:ident) => {
        struct $list_name;
        impl $crate::linked_list::ListNode for $list_name {
            type Elem = $elem;
            fn elem_to_link(elem: Self::Elem) -> core::ptr::NonNull<ListLink<Self>> {
                unsafe {
                    core::ptr::NonNull::new_unchecked(
                        &mut ((*$crate::Static::into_nonnull(elem).as_mut()).$field) as *mut _,
                    )
                }
            }

            fn from_link_to_elem(
                link: core::ptr::NonNull<$crate::linked_list::ListLink<Self>>,
            ) -> Self::Elem {
                let nonnull = Self::from_link_to_nonnull(link);
                unsafe { $crate::Static::from_nonnull(nonnull) }
            }

            fn from_link_to_nonnull(
                link: core::ptr::NonNull<$crate::linked_list::ListLink<Self>>,
            ) -> core::ptr::NonNull<<Self::Elem as $crate::Static>::Inner> {
                let offset =
                    $crate::linked_list::offset_of!(<Self::Elem as $crate::Static>::Inner, $field);
                // SAFETY: It won't be null since link is nonnull.
                unsafe {
                    core::ptr::NonNull::new_unchecked(
                        (link.as_ptr() as *mut u8).offset(-offset) as *mut _
                    )
                }
            }
        }
    };
}

/// An intrusive linked list.
pub struct List<L: ListNode> {
    head: Option<NonNull<ListLink<L>>>,
    tail: Option<NonNull<ListLink<L>>>,
    _pd: PhantomData<L>,
}

impl<L: ListNode> List<L> {
    /// Creates an empty linked list.
    pub const fn new() -> List<L> {
        List {
            head: None,
            tail: None,
            _pd: PhantomData,
        }
    }

    /// Returns `true` if the list is empty. `O(1)`.
    pub fn is_empty(&self) -> bool {
        self.head.is_some()
    }

    /// Returns the number of elements. `O(n)`.
    pub fn len(&self) -> usize {
        let mut len = 0;
        for _ in self.iter() {
            len += 1;
        }
        len
    }

    /// Removes and returns the first element satisfying the predicate `pred`. It
    /// returns `None` the list is empty or `f` returned only `false` on all
    /// elements. `O(n)`.
    pub fn remove_first_if<F>(&mut self, pred: F) -> Option<L::Elem>
    where
        F: Fn(&<L::Elem as Static>::Inner) -> bool,
    {
        let mut current = self.head;
        while let Some(link) = current {
            if pred(unsafe { L::from_link_to_nonnull(link).as_ref() }) {
                self.remove(unsafe { link.as_ref() });
                return Some(L::from_link_to_elem(link));
            }

            current = unsafe { link.as_ref().next() };
        }

        None
    }

    /// Calls the callback for each element. `O(n)`.
    fn walk_links<F, R>(&self, mut f: F) -> Option<R>
    where
        F: FnMut(NonNull<ListLink<L>>) -> ControlFlow<R>,
    {
        let mut current = self.head;
        while let Some(link) = current {
            if let ControlFlow::Break(value) = f(link) {
                return Some(value);
            }

            current = unsafe { link.as_ref().next() };
        }

        None
    }

    /// Inserts an element at the end the list. Returns `Err(elem)` if any other
    /// thread have just inserted the element to a (possibly another) list using
    /// the same link as defined in `L`. `O(1)`.
    pub fn push_back(&mut self, elem: L::Elem) -> Result<(), L::Elem> {
        unsafe {
            let link_ptr = L::elem_to_link(elem);
            let link = link_ptr.as_ref();

            // Prevent multiple threads from inserting the same link at once.
            //
            // Say CPU 1 and CPU 2 are trying adding the thread A to their own
            // runqueues simultaneously:
            //
            //   CPU 1: runqueue1.push_back(thread_A.clone());
            //   CPU 2: runqueue2.push_back(thread_A.clone());
            //
            // In this case, one of the threads (CPU1 or CPU2) fail to insert
            // the element.
            if !link.push_lock.swap(false, Ordering::SeqCst) {
                return Err(L::from_link_to_elem(link_ptr));
            }

            assert!(
                !link.is_in_use(),
                "tried to insert an already inserted link to another list"
            );

            if let Some(tail) = self.tail {
                tail.as_ref().next.set(Some(link_ptr));
            }

            if self.head.is_none() {
                self.head = Some(link_ptr);
            }

            link.prev.set(self.tail);
            link.next.set(None);
            self.tail = Some(link_ptr);
            Ok(())
        }
    }

    /// Pops the element at the beginning of the list. `O(1)`.
    pub fn pop_front(&mut self) -> Option<L::Elem> {
        match self.head {
            Some(head) => unsafe {
                self.remove(head.as_ref());
                Some(L::from_link_to_elem(head))
            },
            None => None,
        }
    }

    pub fn is_link_in_list(&mut self, link: &ListLink<L>) -> bool {
        let elem_nonnull = unsafe { NonNull::new_unchecked(link as *const _ as *mut _) };
        self.walk_links(|link| {
            if link == elem_nonnull {
                ControlFlow::Break(true)
            } else {
                ControlFlow::Continue(())
            }
        })
        .unwrap_or(false)
    }

    /// Removes an element in the list. `O(1)`.
    ///
    /// Caller must make sure that the element is in the list.
    pub fn remove(&mut self, link: &ListLink<L>) {
        // Because we don't need the access to `self`, we can define this
        // method as `List::remove(elem: L::Elem)`. However, since it allows
        // simultaneous removals and it would break links, we intentionally
        // require `&mut self` to prevent such a race.

        // Make sure the element is in the list or this method would mutate other
        // lists.
        debug_assert!(self.is_link_in_list(link));

        match (link.prev.get(), link.next.get()) {
            (Some(prev), Some(next)) => unsafe {
                next.as_ref().prev.set(Some(prev));
                prev.as_ref().next.set(Some(next));
            },
            (None, Some(next)) => unsafe {
                next.as_ref().prev.set(None);
                self.head = Some(next);
            },
            (Some(prev), None) => unsafe {
                prev.as_ref().next.set(None);
                self.tail = Some(prev);
            },
            (None, None) => {
                self.head = None;
                self.tail = None;
            }
        }

        link.prev.set(None);
        link.next.set(None);
        debug_assert!(!link.push_lock.swap(true, Ordering::SeqCst));
    }

    fn iter(&self) -> Iter<'_, L> {
        Iter {
            current: self.head,
            _pd: &PhantomData,
        }
    }
}

impl<L: ListNode> Default for List<L> {
    fn default() -> Self {
        Self::new()
    }
}

pub struct Iter<'a, L: ListNode> {
    current: Option<NonNull<ListLink<L>>>,
    _pd: &'a PhantomData<L>,
}

impl<'a, L: ListNode> Iterator for Iter<'a, L> {
    type Item = &'a <L::Elem as Static>::Inner;
    fn next(&mut self) -> Option<&'a <L::Elem as Static>::Inner> {
        self.current.map(|current| unsafe {
            self.current = current.as_ref().next();
            L::from_link_to_nonnull(current).as_ref()
        })
    }
}

impl<'a, L: ListNode> IntoIterator for &'a List<L> {
    type Item = &'a <L::Elem as Static>::Inner;
    type IntoIter = Iter<'a, L>;

    fn into_iter(self) -> Iter<'a, L> {
        self.iter()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::sync::Arc;

    define_list_node!(MyList, Arc<MyElem>, node);
    #[derive(Debug)]
    struct MyElem {
        value: usize,
        node: ListLink<MyList>,
    }

    #[test]
    pub fn push_and_pop() {
        let mut l: List<MyList> = List::new();
        let elem1 = Arc::new(MyElem {
            value: 123,
            node: Default::default(),
        });
        let elem2 = Arc::new(MyElem {
            value: 456,
            node: Default::default(),
        });

        assert_eq!(l.len(), 0);
        assert!(l.push_back(elem1).is_ok());
        assert_eq!(l.len(), 1);
        assert!(l.push_back(elem2).is_ok());
        assert_eq!(l.len(), 2);

        assert_eq!(l.pop_front().map(|e| e.value), Some(123));
        assert_eq!(l.len(), 1);
        assert_eq!(l.pop_front().map(|e| e.value), Some(456));
        assert_eq!(l.len(), 0);
        assert_eq!(l.pop_front().map(|e| e.value), None);
    }

    fn populate_3_elems() -> (List<MyList>, Arc<MyElem>, Arc<MyElem>, Arc<MyElem>) {
        let mut l: List<MyList> = List::new();
        let elem1 = Arc::new(MyElem {
            value: 1,
            node: Default::default(),
        });
        let elem2 = Arc::new(MyElem {
            value: 20,
            node: Default::default(),
        });
        let elem3 = Arc::new(MyElem {
            value: 300,
            node: Default::default(),
        });
        assert!(l.push_back(elem1.clone()).is_ok());
        assert!(l.push_back(elem2.clone()).is_ok());
        assert!(l.push_back(elem3.clone()).is_ok());
        (l, elem1, elem2, elem3)
    }

    #[test]
    pub fn iter() {
        let mut l: List<MyList> = List::new();
        assert!(l.iter().next().is_none());

        let elem1 = Arc::new(MyElem {
            value: 1,
            node: Default::default(),
        });
        assert!(l.push_back(elem1).is_ok());

        let mut iter = l.iter();
        assert!(iter.next().is_some());
        assert!(iter.next().is_none());
    }

    #[test]
    pub fn remove_elem_at_head() {
        let (mut l, elem1, _elem2, _elem3) = populate_3_elems();
        l.remove(&elem1.node);
        assert_eq!(l.iter().map(|e| e.value).sum::<usize>(), 320);
    }

    #[test]
    pub fn remove_elem_at_middle() {
        let (mut l, _elem1, elem2, _elem3) = populate_3_elems();
        l.remove(&elem2.node);
        assert_eq!(l.iter().map(|e| e.value).sum::<usize>(), 301);
    }

    #[test]
    pub fn remove_elem_at_tail() {
        let (mut l, _elem1, _elem2, elem3) = populate_3_elems();
        l.remove(&elem3.node);
        assert_eq!(l.iter().map(|e| e.value).sum::<usize>(), 21);
    }
}
