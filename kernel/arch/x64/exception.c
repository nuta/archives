#include <kernel/types.h>
#include <kernel/memory.h>
#include <kernel/printk.h>
#include "asm.h"
#include "exception.h"
#include "usercopy.h"

void x64_handle_exception(u8_t exception, u64_t error, u64_t rip) {

    /* We're using IST0's stack. Context switches in an exception
       handling context are prohibited. */
    switch (exception) {
        case EXP_PAGE_FAULT: {
            uptr_t address = asm_read_cr2();
            bool present = (error >> 0) & 1;
            bool write = (error >> 1) & 1;
            bool user = (error >> 2) & 1;
            bool rsvd = (error >> 3) & 1;
            bool exec = (error >> 4) & 1;

            if (rsvd) {
                BUG("page fault: RSVD bit violation");
            }

            if (rip == (u64_t) &x64_do_copy_from_user) {
                DEBUG("PF in usercopy");
                user = true;
            }

            DEBUG("x64: #PF at %p (err=%#x, RIP=%p)",
                address, error, rip);
            handle_page_fault(address, present, user, write, exec);
            break;
        }
        default:
            PANIC("Exception %d", exception);
    }
}
