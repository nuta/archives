use core::slice;

use arrayvec::ArrayVec;
use ftl_utils::alignment::align_up;

use crate::Error;
use crate::dtb::Dtb;
use crate::name::Name;
use crate::prop::PropIter;
use crate::spec::FDT_BEGIN_NODE;
use crate::spec::FDT_END;
use crate::spec::FDT_END_NODE;
use crate::spec::FDT_NOP;
use crate::spec::FDT_PROP;

fn find_parent_bus(parents: &[Parent]) -> Result<(u32, u32), Error> {
    for parent in parents.iter().rev() {
        if let Parent::Bus {
            address_cells,
            size_cells,
            ..
        } = parent
        {
            return Ok((*address_cells, *size_cells));
        }
    }

    Err(Error::MissingParent)
}

pub struct Reg {
    pub addr: u64,
    pub size: u64,
}

pub struct Node<'a> {
    dtb: &'a Dtb<'a>,
    parents: ArrayVec<Parent, 8>,
    name: Name<'a>,
    props_index: usize,
}

impl<'a> Node<'a> {
    pub(crate) fn new(
        dtb: &'a Dtb<'a>,
        parents: ArrayVec<Parent, 8>,
        name: Name<'a>,
        props_index: usize,
    ) -> Self {
        Self {
            dtb,
            parents,
            name,
            props_index,
        }
    }

    pub fn name(&self) -> &[u8] {
        self.name.as_bytes()
    }

    pub fn props(&self) -> PropIter<'a> {
        PropIter::new(self.dtb, self.props_index)
    }

    /// Returns `true` if the node is a reserved memory node.
    pub fn is_reserved_memory_node(&self) -> bool {
        if self.name() == b"reserved-memory" {
            // The node is the reserved-memory node itself, not its children.
            return false;
        }

        self.parents
            .iter()
            .any(|parent| matches!(parent, Parent::Bus { is_reserved_memory, .. } if *is_reserved_memory))
    }

    /// Parses the `reg` property.
    ///
    /// <https://devicetree-specification.readthedocs.io/en/stable/devicetree-basics.html#reg>J
    pub fn reg(&self) -> Result<Option<Reg>, Error> {
        let Some(reg_prop) = self.props().find(|prop| prop.name() == b"reg") else {
            return Ok(None);
        };

        let (address_cells, size_cells) = find_parent_bus(&self.parents)?;
        let u32_array = reg_prop.as_u32_array()?;

        if u32_array.len() != (address_cells + size_cells) as usize {
            return Err(Error::ValueSizeMismatch);
        }

        let mut index = 0;
        let mut addr = 0u64;
        for _ in 0..address_cells {
            addr = (u32_array[index].get() as u64) | (addr << 32);
            index += 1;
        }

        let mut size = 0u64;
        for _ in 0..size_cells {
            size = (u32_array[index].get() as u64) | (size << 32);
            index += 1;
        }

        Ok(Some(Reg { addr, size }))
    }
}

#[derive(Debug, Clone, Copy)]
pub(crate) enum Parent {
    Bus {
        address_cells: u32,
        size_cells: u32,
        is_reserved_memory: bool,
    },
    None,
}

pub struct NodeIter<'a> {
    parents: ArrayVec<Parent, 8>,
    index: usize,
    dtb: &'a Dtb<'a>,
}

impl<'a> NodeIter<'a> {
    pub fn new(dtb: &'a Dtb<'a>) -> Self {
        Self {
            parents: ArrayVec::new(),
            index: 0,
            dtb,
        }
    }
}

impl<'a> Iterator for NodeIter<'a> {
    type Item = Result<Node<'a>, Error>;

    fn next(&mut self) -> Option<Self::Item> {
        // Here's the tree structure described in the specification:
        //
        // > - (optionally) any number of FDT_NOP tokens
        // > - FDT_BEGIN_NODE token
        // >   - The node’s name as a null-terminated string
        // >   - [zeroed padding bytes to align to a 4-byte boundary]
        // >
        // > - For each property of the node:
        // >   - (optionally) any number of FDT_NOP tokens
        // >   - FDT_PROP token
        // >     - property information as given in section 5.4.1
        // >     - [zeroed padding bytes to align to a 4-byte boundary]
        // >
        // > - Representations of all child nodes in this format
        // > - (optionally) any number of FDT_NOP tokens
        // > - FDT_END_NODE token
        // >
        // > <https://devicetree-specification.readthedocs.io/en/stable/flattened-format.html#tree-structure>
        while self.index < self.dtb.structs.len() {
            let token = self.dtb.structs[self.index].get();
            match token {
                // Begin a new node.
                FDT_BEGIN_NODE => {
                    // Skip the token (FDT_BEGIN_NODE).
                    self.index += 1;

                    // Parse the node name.
                    let name_index = self.index;
                    let mut name_len = 0;
                    'outer: loop {
                        if self.index >= self.dtb.structs.len() {
                            return Some(Err(Error::NodeNameOutOfBounds));
                        }

                        let chars = self.dtb.structs[self.index].get();
                        for c in chars.to_be_bytes() {
                            if c == 0 {
                                break 'outer;
                            }

                            name_len += 1;
                        }

                        self.index += 1;
                    }

                    // Advance the index to the values.
                    let aligned_bytes = align_up(name_len + 1, size_of::<u32>());
                    let name_words = aligned_bytes / size_of::<u32>();
                    self.index = name_index + name_words;

                    // Extract the slice of the node name.
                    let name_bytes = unsafe {
                        let ptr = &self.dtb.structs[name_index];
                        let u8_ptr = ptr as *const _ as *const u8;
                        slice::from_raw_parts(u8_ptr, name_len)
                    };

                    // Parse the #address-cells and #size-cells properties if they exist.
                    let mut address_cells = None;
                    let mut size_cells = None;
                    let props = PropIter::new(self.dtb, self.index);
                    for prop in props {
                        match prop.name() {
                            b"#address-cells" => {
                                address_cells = Some(prop.as_u32().unwrap());
                            }
                            b"#size-cells" => {
                                size_cells = Some(prop.as_u32().unwrap());
                            }
                            _ => {}
                        }
                    }

                    if let (Some(address_cells), Some(size_cells)) = (address_cells, size_cells) {
                        self.parents.push(Parent::Bus {
                            is_reserved_memory: name_bytes == b"reserved-memory",
                            address_cells,
                            size_cells,
                        });
                    } else {
                        self.parents.push(Parent::None);
                    }

                    let node = Node::new(
                        self.dtb,
                        self.parents.clone(),
                        Name::new(name_bytes),
                        self.index,
                    );

                    return Some(Ok(node));
                }
                // End the current node.
                FDT_END_NODE => {
                    self.index += 1;
                    self.parents.pop();
                    continue;
                }
                // A node property.
                FDT_PROP => {
                    if self.index + 2 >= self.dtb.structs.len() {
                        return Some(Err(Error::PropOutOfBounds));
                    }

                    let len = self.dtb.structs[self.index + 1].get() as usize;
                    let value_words = align_up(len, size_of::<u32>()) / size_of::<u32>();
                    self.index += 3 + value_words;
                }
                FDT_NOP => {
                    self.index += 1;
                }
                FDT_END => {
                    return None;
                }
                _ => {
                    return Some(Err(Error::UnknownToken(token)));
                }
            }
        }

        None
    }
}
