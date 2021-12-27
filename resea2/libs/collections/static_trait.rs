use core::ptr::NonNull;

/// A trait represents a object whose location won't be changed.
pub trait Static {
    type Inner;
    /// Converts into `NonNull<Self::Inner>`.
    fn into_nonnull(self) -> NonNull<Self::Inner>;
    /// Converts into `NonNull<Self::Inner>`.
    ///
    /// # Safety
    ///
    /// `this` must have been previously return by [`Static::into_nonnull`].
    unsafe fn from_nonnull(this: NonNull<Self::Inner>) -> Self;
}

#[cfg(test)]
use std::sync::Arc;

#[cfg(test)]
impl<T> Static for Arc<T> {
    type Inner = T;
    fn into_nonnull(self) -> NonNull<T> {
        // SAFETY: Arc::into_raw() never returns NULL.
        unsafe { NonNull::new_unchecked(Arc::into_raw(self) as *mut _) }
    }

    unsafe fn from_nonnull(this: NonNull<Self::Inner>) -> Arc<T> {
        Arc::from_raw(this.as_ptr())
    }
}

impl<T> Static for NonNull<T> {
    type Inner = T;
    fn into_nonnull(self) -> NonNull<T> {
        self
    }

    unsafe fn from_nonnull(this: NonNull<Self::Inner>) -> NonNull<T> {
        this
    }
}
