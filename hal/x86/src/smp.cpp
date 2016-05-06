#include <hal.h>


void x86_init_smp (void) {
    struct mp_float_ptr *mpfltptr;
    struct mp_table_header *mptblhdr;
    struct mp_ioapic_entry *ioapic_entry;
    void *entry_ptr;

    // look for MP Floating Pointer Table
    for (uint8_t *p= (uint8_t *) 0xf0000; (uint64_t) p < 0x100000; p++) {
        if (p[0]=='_' && p[1] == 'M' && p[2] == 'P' && p[3]=='_') {
            mpfltptr = (struct mp_float_ptr *) p;
            break;
        }
    }

    if (mpfltptr == nullptr)
        return;

    mptblhdr = (struct mp_table_header *)
               ((paddr_t) mpfltptr->mptable_header_addr);

    if (
        mptblhdr->signature[0] != 'P' || mptblhdr->signature[1] != 'C' ||
        mptblhdr->signature[2] != 'M' || mptblhdr->signature[3] != 'P'
    )
        return;

    entry_ptr = (void *) ((paddr_t) mptblhdr + sizeof(struct mp_table_header));

    for (int i=0; i < mptblhdr->entry_count; i++) {
        switch(*((uint8_t *) entry_ptr)) {

            /*
             *  I/O APIC
             *
             */
            case MP_BASETABLE_IOAPIC_ENTRY:
                ioapic_entry = (struct mp_ioapic_entry *) entry_ptr;
                entry_ptr = (void *) ((paddr_t) entry_ptr + (paddr_t) sizeof(struct mp_ioapic_entry));

                // enable the I/O APIC
                if (ioapic_entry->ioapic_flags != 0)
                    x86_init_ioapic(ioapic_entry->memmaped_ioapic_addr);
                break;

            case MP_BASETABLE_BUS_ENTRY:
                entry_ptr = (void *) ((paddr_t) entry_ptr + (paddr_t) sizeof(struct mp_bus_entry));
                break;
            case MP_BASETABLE_PROCESSOR_ENTRY:
                entry_ptr = (void *) ((paddr_t) entry_ptr + (paddr_t) sizeof(struct mp_processor_entry));
                break;
            case MP_BASETABLE_IOINT_ASSIGN_ENTRY:
                entry_ptr = (void *) ((paddr_t) entry_ptr + (paddr_t) sizeof(struct mp_ioint_assign_entry));
                break;
            case MP_BASETABLE_LOCALINT_ASSIGN_ENTRY:
                entry_ptr = (void *) ((paddr_t) entry_ptr + (paddr_t) sizeof(struct mp_localint_assign_entry));
                break;
        }
    }

    return;
}
