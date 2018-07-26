#include "memory.h"
#include "printk.h"
#include "test.h"

void handle_irq(int irq) {
}

#define assert_eq(desc, e1, e2)                                                      \
    do {                                                                             \
        INFO("[%s:%d] test: %s", __func__, __LINE__, desc);                          \
        __typeof__(e1) _e1 = e1;                                                     \
        __typeof__(e2) _e2 = e2;                                                     \
        if (_e1 != _e2) {                                                            \
            INFO("test failed: expected " #e1 " (%d) == " #e2 " (%d)", _e1, _e2);    \
        }                                                                            \
    } while(0)

void kernel_test(void) {
    size_t allocated = get_allocated_pages();

    paddr_t addr = alloc_pages(GET_PAGE_NUM(PAGE_SIZE), KMALLOC_NORMAL);
    assert_eq("All pages should be freed.", allocated, get_allocated_pages());
    PANIC("Finished all tests.");
}
