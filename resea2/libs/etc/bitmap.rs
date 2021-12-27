pub struct BitMap<'a> {
    bitmap: &'a mut [u8],
    len: usize,
}

#[allow(clippy::len_without_is_empty)]
impl<'a> BitMap<'a> {
    pub fn new(bitmap: &'a mut [u8], len: usize) -> BitMap<'a> {
        BitMap { bitmap, len }
    }

    pub fn len(&self) -> usize {
        self.len
    }

    pub fn inner(&self) -> &[u8] {
        self.bitmap
    }

    #[inline(always)]
    pub fn try_get(&self, index: usize) -> Option<bool> {
        if index >= self.len() {
            return None;
        }

        Some(self.get(index))
    }

    #[inline(always)]
    pub fn get(&self, index: usize) -> bool {
        debug_assert!(index < self.len());
        self.bitmap[index / 8] & (1 << (index % 8)) != 0
    }

    #[inline(always)]
    pub fn set(&mut self, index: usize) {
        self.bitmap[index / 8] |= 1 << (index % 8);
    }

    #[inline(always)]
    pub fn unset(&mut self, index: usize) {
        self.bitmap[index / 8] &= !(1 << (index % 8));
    }

    #[inline(always)]
    pub fn set_contiguous(&mut self, index: usize, n: usize) {
        for i in 0..n {
            self.set(index + i)
        }
    }

    #[inline(always)]
    pub fn unset_contiguous(&mut self, index: usize, n: usize) {
        for i in 0..n {
            self.unset(index + i)
        }
    }

    /// Looks for a contiguous `len` zeros.
    pub fn find_contiguous_zeros(&self, len: usize) -> Option<usize> {
        if len == 0 {
            return None;
        }

        let mut start = self.len;
        for i in 0..self.bitmap.len() {
            if self.bitmap[i] != 0xff {
                // Found the first zero in the bitmap.
                start = i * 8;
                break;
            }
        }

        for base in start..self.len.saturating_sub(len - 1) {
            if (0..len).all(|i| !self.get(base + i)) {
                return Some(base);
            }
        }

        None
    }
}

#[cfg(all(test, not(feature = "no_std")))]
mod tests {
    use super::*;

    #[test]
    fn set_and_unset_contiguous() {
        let mut array = [0, 0];
        let mut bitmap = BitMap::new(&mut array, 12);
        bitmap.set_contiguous(4, 8);
        assert_eq!(bitmap.inner(), &[0b1111_0000, 0b0000_1111]);
        bitmap.unset_contiguous(6, 2);
        assert_eq!(bitmap.inner(), &[0b0011_0000, 0b0000_1111]);
    }

    #[test]
    fn find_contiguous_zeros() {
        let mut array = [0b1111_1101, 0b0000_0001];
        let bitmap = BitMap::new(&mut array, 12);
        assert_eq!(bitmap.find_contiguous_zeros(1), Some(1));
        assert_eq!(bitmap.find_contiguous_zeros(3), Some(9));
        assert_eq!(bitmap.find_contiguous_zeros(4), None);
    }
}
