use core::slice;

use ftl_utils::alignment::align_up;
pub use ftl_utils::endianess::BE;

use crate::Error;
use crate::dtb::Dtb;
use crate::name::Name;
use crate::spec::FDT_PROP;

pub struct Prop<'a> {
    name: Name<'a>,
    value: &'a [u8],
}

impl<'a> Prop<'a> {
    pub fn name(&self) -> &[u8] {
        self.name.as_bytes()
    }

    pub fn as_u32_array(&self) -> Result<&[BE<u32>], Error> {
        if !self.value.len().is_multiple_of(size_of::<u32>()) {
            return Err(Error::ValueSizeNotAligned);
        }

        let n = self.value.len() / size_of::<u32>();
        Ok(unsafe {
            let ptr = self.value.as_ptr() as *const BE<u32>;
            slice::from_raw_parts(ptr, n)
        })
    }

    pub fn as_u32(&self) -> Result<u32, Error> {
        if self.value.len() != size_of::<u32>() {
            return Err(Error::ValueSizeMismatch);
        }

        let value = unsafe {
            let ptr = self.value.as_ptr() as *const BE<u32>;
            (*ptr).get()
        };

        Ok(value)
    }

    pub fn as_cstr(&self) -> Result<&[u8], Error> {
        for (i, b) in self.value.iter().enumerate() {
            if *b == 0 {
                return Ok(&self.value[..i]);
            }
        }

        Err(Error::NotNullTerminated)
    }

    pub fn as_bytes(&self) -> &[u8] {
        self.value
    }
}

pub struct PropIter<'a> {
    index: usize,
    dtb: &'a Dtb<'a>,
}

impl<'a> PropIter<'a> {
    pub fn new(dtb: &'a Dtb<'a>, index: usize) -> Self {
        Self { index, dtb }
    }
}

impl<'a> Iterator for PropIter<'a> {
    type Item = Prop<'a>;

    fn next(&mut self) -> Option<Self::Item> {
        if self.index + 1 >= self.dtb.structs.len() {
            return None;
        }

        let token = self.dtb.structs[self.index].get();
        if token != FDT_PROP {
            return None;
        }

        self.index += 1;
        let len = self.dtb.structs[self.index].get();
        self.index += 1;
        let nameoff = self.dtb.structs[self.index].get() as usize;

        let name_rest = &self.dtb.strings[nameoff..];
        let mut name_len = 0;
        for c in name_rest {
            if *c == 0 {
                break;
            }

            name_len += 1;
        }

        self.index += 1;

        let struct_rest_u32 = &self.dtb.structs[self.index..];
        let struct_rest_bytes = unsafe {
            let ptr = struct_rest_u32.as_ptr() as *const u8;
            slice::from_raw_parts(ptr, struct_rest_u32.len() * size_of::<u32>())
        };

        let raw_value = &struct_rest_bytes[..len as usize];

        let value_words = align_up(len as usize, size_of::<u32>()) / size_of::<u32>();
        self.index += value_words;

        Some(Prop {
            name: Name::new(&name_rest[..name_len]),
            value: raw_value,
        })
    }
}
