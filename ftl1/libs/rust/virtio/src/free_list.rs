pub use ftl::prelude::*;

pub struct FreeList<T> {
    items: Vec<T>,
}

impl<T> Default for FreeList<T> {
    fn default() -> Self {
        Self::new()
    }
}

impl<T> FreeList<T> {
    pub fn new() -> Self {
        Self { items: Vec::new() }
    }

    pub fn push(&mut self, item: T) {
        self.items.push(item);
    }

    pub fn pop(&mut self, n: usize) -> Option<Vec<T>> {
        if self.items.len() < n {
            return None;
        }

        let items = self.items.split_off(self.items.len() - n);
        Some(items)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_pop() {
        let mut free_list = FreeList::new();
        free_list.push(1);
        free_list.push(2);
        free_list.push(3);

        let items = free_list.pop(2);
        assert_eq!(items, Some(vec![2, 3]));

        let items = free_list.pop(1);
        assert_eq!(items, Some(vec![1]));

        let items = free_list.pop(1);
        assert_eq!(items, None);
    }
}
