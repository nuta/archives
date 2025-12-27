//! Generic slot allocator for managing fixed-size resource pools.
//!
//! Common use case: device drivers need to manage fixed-size DMA buffer pools
//! where each buffer must be tracked until the device completes using it.
//!
//! # Examples
//!
//! ```
//! use ftl_utils::slot_allocator::SlotAllocator;
//!
//! // Allocate slots from a pool of 256
//! let mut allocator = SlotAllocator::<usize>::new(256);
//!
//! // Allocate a slot
//! let slot_idx = allocator.alloc().unwrap();
//! assert_eq!(slot_idx, 255);
//!
//! // Track it with a key (e.g., DMA address)
//! let key = 0x1000;
//! allocator.track(key, slot_idx);
//!
//! // Later, retrieve and free by key
//! if let Some(slot_idx) = allocator.untrack(&key) {
//!     allocator.free(slot_idx);
//! }
//! ```

use alloc::vec::Vec;

/// Generic slot allocator for managing a fixed-size pool of slots.
///
/// Generic over the key type `K` used to track allocated slots.
/// Common choices: `usize`, `BusAddr`, or any type that implements `Eq + Copy`.
pub struct SlotAllocator<K> {
    free_slots: Vec<usize>,
    /// Tracks in-use slots: (key, slot_index)
    tracked: Vec<(K, usize)>,
}

impl<K> SlotAllocator<K>
where
    K: Eq + Copy,
{
    /// Create a new slot allocator with `capacity` slots.
    ///
    /// All slots are initially free (0..capacity).
    pub fn new(capacity: usize) -> Self {
        Self {
            free_slots: (0..capacity).collect(),
            tracked: Vec::new(),
        }
    }

    /// Allocate a slot from the free pool.
    ///
    /// Returns `None` if no slots are available.
    pub fn alloc(&mut self) -> Option<usize> {
        self.free_slots.pop()
    }

    /// Free a slot, returning it to the pool.
    ///
    /// # Panics
    ///
    /// Panics in debug builds if the slot is still tracked.
    pub fn free(&mut self, slot_idx: usize) {
        debug_assert!(
            !self.is_tracked_by_slot(slot_idx),
            "Freeing tracked slot {}",
            slot_idx
        );
        self.free_slots.push(slot_idx);
    }

    /// Track an allocated slot with a key.
    ///
    /// This allows later retrieval via `untrack()` or `get_slot()`.
    ///
    /// # Panics
    ///
    /// Panics in debug builds if the key is already tracked.
    pub fn track(&mut self, key: K, slot_idx: usize) {
        debug_assert!(!self.is_tracked_by_key(&key), "Key already tracked");
        self.tracked.push((key, slot_idx));
    }

    /// Untrack a slot by key and return its index.
    ///
    /// Returns `None` if the key is not found.
    ///
    /// Note: This does NOT free the slot - you must call `free()` separately.
    pub fn untrack(&mut self, key: &K) -> Option<usize> {
        let pos = self.tracked.iter().position(|(k, _)| k == key)?;
        let (_, slot_idx) = self.tracked.swap_remove(pos);
        Some(slot_idx)
    }

    /// Get the slot index for a tracked key without untracking it.
    pub fn get_slot(&self, key: &K) -> Option<usize> {
        self.tracked
            .iter()
            .find(|(k, _)| k == key)
            .map(|(_, slot_idx)| *slot_idx)
    }

    /// Check if a key is currently tracked.
    pub fn is_tracked_by_key(&self, key: &K) -> bool {
        self.tracked.iter().any(|(k, _)| k == key)
    }

    /// Check if a slot is currently tracked.
    pub fn is_tracked_by_slot(&self, slot_idx: usize) -> bool {
        self.tracked.iter().any(|(_, idx)| *idx == slot_idx)
    }

    /// Number of free slots available.
    pub fn available(&self) -> usize {
        self.free_slots.len()
    }

    /// Number of tracked (in-use) slots.
    pub fn in_use(&self) -> usize {
        self.tracked.len()
    }

    /// Total capacity.
    pub fn capacity(&self) -> usize {
        self.free_slots.len() + self.tracked.len()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_alloc_free() {
        let mut alloc = SlotAllocator::<usize>::new(4);
        assert_eq!(alloc.available(), 4);

        let slot0 = alloc.alloc().unwrap();
        let slot1 = alloc.alloc().unwrap();
        assert_eq!(alloc.available(), 2);

        alloc.free(slot0);
        alloc.free(slot1);
        assert_eq!(alloc.available(), 4);
    }

    #[test]
    fn test_track_untrack() {
        let mut alloc = SlotAllocator::<usize>::new(4);

        let slot = alloc.alloc().unwrap();
        alloc.track(100, slot);

        assert_eq!(alloc.get_slot(&100), Some(slot));
        assert!(alloc.is_tracked_by_key(&100));
        assert!(alloc.is_tracked_by_slot(slot));

        let retrieved = alloc.untrack(&100).unwrap();
        assert_eq!(retrieved, slot);
        assert!(!alloc.is_tracked_by_key(&100));

        alloc.free(slot);
        assert_eq!(alloc.available(), 4);
    }

    #[test]
    fn test_exhaustion() {
        let mut alloc = SlotAllocator::<usize>::new(2);

        let slot0 = alloc.alloc().unwrap();
        let _slot1 = alloc.alloc().unwrap();
        assert!(alloc.alloc().is_none());

        alloc.free(slot0);
        assert!(alloc.alloc().is_some());
    }

    #[test]
    fn test_capacity() {
        let alloc = SlotAllocator::<usize>::new(10);
        assert_eq!(alloc.capacity(), 10);

        let mut alloc = SlotAllocator::<usize>::new(10);
        let slot = alloc.alloc().unwrap();
        alloc.track(42, slot);

        assert_eq!(alloc.available(), 9);
        assert_eq!(alloc.in_use(), 1);
        assert_eq!(alloc.capacity(), 10);
    }
}
