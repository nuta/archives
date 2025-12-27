use core::slice;

use ftl_utils::endianess::BE;

use crate::Error;
use crate::node::NodeIter;
use crate::spec::*;

fn parse_header(dtb: *const u8) -> Result<usize, Error> {
    let header = unsafe { &*(dtb as *const Header) };
    if header.magic.get() != FDT_MAGIC {
        return Err(Error::InvalidMagic);
    }

    let total_size = header.totalsize.get() as usize;
    Ok(total_size)
}

pub struct Dtb<'a> {
    len: usize,
    pub(crate) structs: &'a [BE<u32>],
    pub(crate) strings: &'a [u8],
}

impl<'a> Dtb<'a> {
    /// # Safety
    ///
    /// The caller must ensure `dtb` points to a valid device tree blob that is readable for the
    /// reported total size.
    pub unsafe fn parse_from_ptr(dtb: *const u8) -> Result<Dtb<'a>, Error> {
        let total_size = parse_header(dtb)?;
        let slice = unsafe { slice::from_raw_parts(dtb, total_size) };
        Self::parse(slice)
    }

    pub fn parse(dtb: &'a [u8]) -> Result<Dtb<'a>, Error> {
        if dtb.len() < size_of::<Header>() {
            return Err(Error::TooShort);
        }

        let total_size = parse_header(dtb.as_ptr())?;
        if total_size > dtb.len() {
            return Err(Error::ShorterThanTotalSize);
        }

        Self::do_parse(dtb)
    }

    fn do_parse(dtb: &'a [u8]) -> Result<Dtb<'a>, Error> {
        let header = unsafe { &*(dtb.as_ptr() as *const Header) };
        if header.magic.get() != FDT_MAGIC {
            return Err(Error::InvalidMagic);
        }

        let totalsize = header.totalsize.get();
        if dtb.len() < totalsize as usize {
            return Err(Error::ShorterThanTotalSize);
        }

        let struct_off = header.off_dt_struct.get() as usize;
        let struct_size = header.size_dt_struct.get() as usize;
        let struct_block = unsafe {
            let ptr = dtb.as_ptr().add(struct_off);
            if ptr.align_offset(size_of::<u32>()) != 0 {
                return Err(Error::NotAligned);
            }

            let n = struct_size / size_of::<u32>();
            slice::from_raw_parts(ptr as *const BE<u32>, n)
        };

        let strings_off = header.off_dt_strings.get() as usize;
        let strings_size = header.size_dt_strings.get() as usize;
        let strings_block = &dtb[strings_off..strings_off + strings_size];

        Ok(Self {
            len: totalsize as usize,
            structs: struct_block,
            strings: strings_block,
        })
    }

    /// Returns the dtb length in bytes.
    pub fn len(&self) -> usize {
        self.len
    }

    pub fn is_empty(&self) -> bool {
        self.len == 0
    }

    pub fn nodes(&self) -> NodeIter<'_> {
        NodeIter::new(self)
    }
}

#[cfg(test)]
mod tests {
    use std::sync::LazyLock;

    use super::*;

    static SIMPLE_FDT: LazyLock<Vec<u8>> = LazyLock::new(|| {
        let mut fdt = vm_fdt::FdtWriter::new().unwrap();
        let root_node = fdt.begin_node("root").unwrap();
        fdt.property_string("compatible", "simple-device").unwrap();
        fdt.end_node(root_node).unwrap();
        fdt.finish().unwrap()
    });

    #[test]
    fn test_parse() {
        let parser = Dtb::parse(SIMPLE_FDT.as_slice()).unwrap();
        let mut nodes = parser.nodes();

        let node = nodes.next().unwrap().unwrap();
        assert_eq!(node.name(), b"root");

        let mut props = node.props();
        let prop = props.next().unwrap();
        assert_eq!(prop.name(), b"compatible");

        assert!(nodes.next().is_none());
    }
}
