#ifndef __X64_SMP_H__
#define __X64_SMP_H__

#include <kernel/types.h>

#define MP_BASETABLE_PROCESSOR_ENTRY        0
#define MP_BASETABLE_BUS_ENTRY              1
#define MP_BASETABLE_IOAPIC_ENTRY           2
#define MP_BASETABLE_IOINT_ASSIGN_ENTRY     3
#define MP_BASETABLE_LOCALINT_ASSIGN_ENTRY  4

struct mp_float_ptr {
    u8_t  signature[4];  // "_MP_"
    u32_t mptable_header_addr;
    u8_t  length;
    u8_t  spec_rev;
    u8_t  checksum;
    u8_t  info1;
    u8_t  info2;
    u8_t  info3[3];
} PACKED;

struct mp_table_header {
    u8_t  signature[4]; // "PCMP"
    u16_t base_table_length;
    u8_t  spec_rev;
    u8_t  checksum;
    u8_t  oem_id[8];
    u8_t  product_id[12];
    u32_t oem_table_pointer;
    u16_t oem_table_size;
    u16_t entry_count;
    u32_t memmaped_localapic_addr;
    u16_t extended_table_length;
    u8_t  extended_table_checksum;
    u8_t  reserved;
} PACKED;

struct mp_processor_entry {
    u8_t  type;          // 0
    u8_t  localapic_id;
    u8_t  localapic_ver;
    u8_t  cpu_flags;
    u32_t cpu_signature;
    u32_t feature_flags;
    u32_t reserved1;
    u32_t reserved2;
} PACKED;

struct mp_bus_entry{
    u8_t  type;          // 1
    u8_t  id;
    u8_t  type_str[6];
} PACKED;

struct mp_ioapic_entry {
    u8_t  type;         // 2
    u8_t  ioapic_id;
    u8_t  ioapic_ver;
    u8_t  ioapic_flags;
    u32_t memmaped_ioapic_addr;
} PACKED;

struct mp_ioint_assign_entry {
    u8_t  type;         // 3
    u8_t  int_type;
    u16_t int_flags;
    u8_t  src_bus_id;
    u8_t  src_bus_irq;
    u8_t  dest_ioapic_id;
    u8_t  dest_ioapic_intin;
} PACKED;

struct mp_localint_assign_entry {
    u8_t  type;         // 4
    u8_t  int_type;
    u16_t int_flags;
    u8_t  src_bus_id;
    u8_t  src_bus_irq;
    u8_t  dest_localapic_id;
    u8_t  dest_localapic_intin;
} PACKED;

void x64_init_smp(void);

#endif
