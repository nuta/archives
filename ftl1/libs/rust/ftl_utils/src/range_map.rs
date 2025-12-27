use alloc::vec::Vec;
use core::ops::Range;

struct Entry<K, V> {
    range: Range<K>,
    value: V,
}

/// A map that maps ranges to values.
///
/// No overlapping ranges are allowed.
pub struct RangeMap<K, V> {
    entries: Vec<Entry<K, V>>,
}

impl<K, V> RangeMap<K, V> {
    pub fn new() -> Self {
        RangeMap {
            entries: Vec::new(),
        }
    }
}

impl<K, V> RangeMap<K, V>
where
    K: PartialOrd + PartialEq + Clone + Copy + Into<usize> + From<usize>,
{
    pub fn overlaps_with(&self, range: &Range<K>) -> bool {
        self.entries
            .iter()
            .any(|e| e.range.start < range.end && e.range.end > range.start)
    }

    /// Inserts a value at the specified range.
    pub fn insert(&mut self, range: Range<K>, value: V) -> Result<(), V> {
        if range.start >= range.end {
            return Err(value);
        }

        if self.overlaps_with(&range) {
            return Err(value);
        }

        self.entries.push(Entry { range, value });
        Ok(())
    }

    /// Inserts a value into any available space in the map.
    pub fn insert_anywhere(
        &mut self,
        size: usize,
        within: Range<K>,
        value: V,
    ) -> Result<Range<K>, V> {
        if let Some(space) = self.find_space(size, within) {
            self.insert(space.clone(), value)?;
            Ok(space)
        } else {
            Err(value)
        }
    }

    fn find_space(&self, size: usize, within: Range<K>) -> Option<Range<K>> {
        if size == 0 {
            return None;
        }

        let mut prev_end: usize = within.start.into();
        let within_usize: Range<usize> = within.start.into()..within.end.into();
        for entry in &self.entries {
            let start: usize = entry.range.start.into();
            if !within_usize.contains(&start) {
                continue;
            }

            let gap_size = start - prev_end;
            if gap_size >= size && prev_end + size <= within_usize.end {
                return Some(Range {
                    start: prev_end.into(),
                    end: (prev_end + size).into(),
                });
            }

            prev_end = entry.range.end.into();
        }

        if let Some(remaining_size) = within_usize.end.checked_sub(prev_end)
            && remaining_size >= size
        {
            return Some(Range {
                start: prev_end.into(),
                end: (prev_end + size).into(),
            });
        }

        None
    }

    pub fn get(&self, key: K) -> Option<(&Range<K>, &V)> {
        self.entries
            .iter()
            .find(|e| e.range.start <= key && key < e.range.end)
            .map(|e| (&e.range, &e.value))
    }
}

impl<K, V> Default for RangeMap<K, V> {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_insert_and_get() {
        let mut map = RangeMap::new();
        map.insert(Range { start: 0, end: 10 }, "test").unwrap();
        assert_eq!(map.get(0), Some((&Range { start: 0, end: 10 }, &"test")));
        assert_eq!(map.get(1), Some((&Range { start: 0, end: 10 }, &"test")));
        assert_eq!(map.get(9), Some((&Range { start: 0, end: 10 }, &"test")));
        assert_eq!(map.get(10), None);
    }

    #[test]
    fn test_insert_anywhere() {
        let mut map = RangeMap::new();
        assert_eq!(
            map.insert_anywhere(10, Range { start: 0, end: 30 }, "test00"),
            Ok(Range { start: 0, end: 10 })
        );
        assert_eq!(
            map.insert_anywhere(10, Range { start: 0, end: 30 }, "test10"),
            Ok(Range { start: 10, end: 20 })
        );
        assert_eq!(
            map.insert_anywhere(10, Range { start: 0, end: 30 }, "test20"),
            Ok(Range { start: 20, end: 30 })
        );
        assert_eq!(
            map.insert_anywhere(10, Range { start: 0, end: 30 }, "should_fail"),
            Err("should_fail")
        );
    }

    #[test]
    fn test_insert_anywhere_after_insert() {
        let mut map = RangeMap::new();
        assert_eq!(map.insert(Range { start: 10, end: 20 }, "test10"), Ok(()));
        assert_eq!(
            map.insert_anywhere(10, Range { start: 0, end: 30 }, "test00"),
            Ok(Range { start: 0, end: 10 })
        );
    }
}
