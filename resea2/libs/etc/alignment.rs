pub const fn align_down(value: usize, align: usize) -> usize {
    (value) & !(align - 1)
}

pub const fn align_up(value: usize, align: usize) -> usize {
    align_down(value + align - 1, align)
}

pub const fn is_aligned(value: usize, align: usize) -> bool {
    value & (align - 1) == 0
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_is_aligned() {
        assert!(!is_aligned(31, 32));
        assert!(is_aligned(32, 32));
        assert!(is_aligned(64, 32));
    }
}
