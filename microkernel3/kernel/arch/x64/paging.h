#ifndef __X64_PAGING_H__
#define __X64_PAGING_H__

// the number of entries of PML4, PDPT, PD and PT are same: 512
#define PAGE_ENTRY_NUM 512
#define PAGE_ALT_SIZE (1 << 7)
#define PAGE_ATTRS 0x6

#define PD_INDEX(vaddr)   (((vaddr) >> 21) & 0x1ff)
#define PDPT_INDEX(vaddr) (((vaddr) >> 30) & 0x1ff)
#define PML4_INDEX(vaddr) (((vaddr) >> 39) & 0x1ff)

void x64_init_paging(void);

#endif
