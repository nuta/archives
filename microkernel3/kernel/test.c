#include "memory.h"
#include "process.h"
#include "thread.h"
#include "ipc.h"
#include "printk.h"
#include "test.h"

void handle_irq(int irq) {
}

#define assert_eq(desc, e1, e2)                                                      \
    do {                                                                             \
        __typeof__(e1) _e1 = e1;                                                     \
        __typeof__(e2) _e2 = e2;                                                     \
        if (_e1 == _e2) {                                                            \
            INFO("[%s:%d] PASS: %s", __func__, __LINE__, desc);                      \
        } else {                                                                     \
            WARN("[%s:%d] FAIL: expected " #e1 " (%d) == " #e2 " (%d)",              \
                 __func__, __LINE__, _e1, _e2);                                      \
        }                                                                            \
    } while(0)

void memory_test(void) {
    size_t allocated = get_allocated_pages();

    /* alloc_pages test */
    paddr_t addr = alloc_pages(8, KMALLOC_NORMAL);
    free_pages(addr, 8);
    assert_eq("free_pages() frees pages.", allocated, get_allocated_pages());
}

struct channel *thread_ch;
void thread1_func(uptr_t thread_arg) {
    error_t err = ipc_send(thread_ch->cid, 0, thread_arg, 1, 0, 0);
    assert_eq("ipc_send() returns ERROR_NONE", err, ERROR_NONE);
    INFO("Time to send");
    err = ipc_send(thread_ch->cid, 0, thread_arg, 2, 0, 0);
    assert_eq("ipc_send() returns ERROR_NONE", err, ERROR_NONE);
    thread_destroy_current();
}

void ipc_test(void) {
    /* process test */
    int thread_arg = 0xabcd1234;
    thread_ch = channel_create(kernel_process);
    struct channel *our_ch = channel_create(kernel_process);
    channel_link(our_ch, thread_ch);
    struct thread *thread1 = thread_create(kernel_process, (uptr_t) thread1_func, thread_arg);
    thread_resume(thread1);

    channel_t from;
    payload_t a0, a1, a2, a3;
    header_t header;

    header = ipc_recv(our_ch->cid, &from, &a0, &a1, &a2, &a3);
    assert_eq("ipc_recv() returns ERROR_NONE", ERRTYPE(header), ERROR_NONE);
    assert_eq("ipc_recv() set correct `from'", from, our_ch->cid);
    assert_eq("ipc_recv() returns correct `a0'", a0, thread_arg);
    assert_eq("ipc_recv() returns correct `a1'", a1, 1);

    header = ipc_recv(our_ch->cid, &from, &a0, &a1, &a2, &a3);
    assert_eq("ipc_recv() returns ERROR_NONE", ERRTYPE(header), ERROR_NONE);
    assert_eq("ipc_recv() set correct `from'", from, our_ch->cid);
    assert_eq("ipc_recv() returns correct `a0'", a0, thread_arg);
    assert_eq("ipc_recv() returns correct `a1'", a1, 2);
}

static void kernel_test_thread(void) {
    INFO("Starting kernel tests...");
    memory_test();
    ipc_test();
    PANIC("Finished all tests.");
}

void kernel_test(void) {
    thread_resume(thread_create(kernel_process, (uptr_t) kernel_test_thread, 0));
}
