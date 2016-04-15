#include <hal.h>
#include <resea.h>
#include <resea/cpp/memory.h>
#include <string.h>
#include "kernel.h"


static struct thread threads[THREAD_NUM_MAX];
static struct thread_group thread_groups[THREAD_NUM_MAX];
static mutex_t lock = MUTEX_UNLOCKED;
static bool started_threading = false;


/**
 *  Returns a human-friendly description of a thread status
 *
 *  @param[in] status  The thread status.
 *  @return  A thread status string.
 *
 */
static const char *get_thread_status_str(int status) {

    switch (status) {
    case THREAD_UNUSED:   return "UNUSED";
    case THREAD_BLOCKED:  return "BLOCKED";
    case THREAD_RUNNABLE: return "RUNNABLE";
    default:
        BUG("invalid thread status");
        return "INVALID";
    }
}


/**
 *  Picks a next thread to run
 *
 *  This searches the thread list for a runnable thread. The
 *  scheduling algorithm is roundrobin. If no thread is runnable, It
 *  halts CPU and waits for an hardware interrupt.
 *
 *  @return  The thread ID of the next thread.
 */
static ident_t get_next_thread(void) {
    static ident_t next = 1;
    ident_t i;

    for (i=next; i < THREAD_NUM_MAX; i++) {
        if (threads[i].status == THREAD_RUNNABLE)
            goto success;
    }

    /* from the beginning */
    for (i=1; i < THREAD_NUM_MAX; i++) {
        if (threads[i].status == THREAD_RUNNABLE)
            goto success;
    }

    INFO("no thread to run");
    for (;;) {
        hal_wait_interrupt();
    }

success:
    next = i + 1;
    return i;
}


/**
 *  Allocates a thread ID
 *
 *  @return  A thread ID on success or 0 on failure.
 */
static ident_t alloc_thread_id(void) {
    ident_t i;

    lock_mutex(&lock);
    for (i=1; i < THREAD_NUM_MAX; i++) {
        if (threads[i].status == THREAD_UNUSED)
            break;
    }

    if (threads[i].status == THREAD_UNUSED) {
        threads[i].status  = THREAD_BLOCKED;
    } else {
        BUG("alloc_thread_id(): failed to allocate a thread ID");
        i = 0;
    }

    unlock_mutex(&lock);
    return i;
}


/**
 *  Allocates a thread group ID
 *
 *  @return  A thread group ID on success or 0 on failure.
 */
static ident_t alloc_thread_group_id(void) {
    ident_t i;

    lock_mutex(&lock);

    for (i=1; i < THREAD_NUM_MAX; i++) {
        if (!thread_groups[i].used)
            break;
    }

    if (thread_groups[i].used) {
        BUG("alloc_thread_group_id(): failed to allocate a thread_gruop struct");
        return 0;
    }

    thread_groups[i].used = true;
    unlock_mutex(&lock);
    return i;
}


/**
 *  Creates a new thread group
 *
 *  @return  A thread group ID on success or 0 on failure.
 */
static ident_t create_thread_group(void) {
    ident_t id;
    struct thread_group *g;

    if ((id = alloc_thread_group_id()) == 0)
        return 0;

    g = kernel_get_thread_group(id);
    g->id                = id;
    g->vm.dynamic_vpages = kernel_allocate_memory(kernel_get_dynamic_vpages_num(), MEMORY_ALLOC_NORMAL);
    g->vm.areas_num      = 0;
    g->num               = 0;

    hal_create_vm_space(&g->vm);
    init_mutex(&g->lock, MUTEX_UNLOCKED);
    init_mutex(&g->vm.dynamic_vpages_lock, MUTEX_UNLOCKED);

    INFO("created a new thread group #%d", id);
    return id;
}


/**
 *  Get the thread struct of the current thread
 *
 *  @return  The thread struct of the current thread.
 */
struct thread *kernel_get_current_thread(void) {

    return &threads[hal_get_current_thread_id()];
}


/**
 *  Get the thread_group struct by a thread group ID
 *
 *  @return  The thread_group struct of the ID.
 */
struct thread_group *kernel_get_thread_group(ident_t group) {

    return &thread_groups[group];
}


/**
 *  Get the thread_group struct by a thread ID
 *
 *  @return  The thread_group struct of a thread group the thread belongs to.
 */
struct thread_group *kernel_get_thread_group_of(ident_t thread) {

    return threads[thread].group;
}


/**
 *  Get the current thread_group struct
 *
 *  @return  The current thread_group struct.
 */
struct thread_group *kernel_get_current_thread_group(void) {

    return kernel_get_thread_group_of(hal_get_current_thread_id());
}


/**
 *  Updates the status of a thread
 *
 *  @param[in] thread  The thread.
 *  @param[in] status  The new status.
 *  @return  A result code.
 *
 */
result_t kernel_set_thread_status(ident_t thread, int status) {

    INFO("thread #%d status changed to %s", thread, get_thread_status_str(status));
    threads[thread].status = status;
    return OK;
}


/**
 *  Set thread registers states
 *
 *  @param[in] thread      The thread to be manipulated.
 *  @param[in] entry       The entry point.
 *  @param[in] arg         The first argument.
 *  @param[in] stack       The beginning of stack.
 *  @param[in] stack_size  The size of stack.
 *  @return  A result code.
 */
result_t kernel_set_thread(ident_t thread, uintptr_t entry, uintptr_t arg,
                       uintptr_t stack, size_t stack_size) {

    BUG_IF(THREAD_NUM_MAX < thread ||
           threads[thread].status == THREAD_UNUSED,
           "invalid thread ID");

    hal_set_thread(&threads[thread].hal, threads[thread].group->id == 1,
                   entry, arg, stack, stack_size);

    return OK;
}


/**
 *  Creates a new thread
 *
 *  @param[in] group       The thread group the new thread will belongs to. If it's 0,
                           new thread group will be created.
 *  @param[in] name        The name.
 *  @param[in] name_size   The size of name.
 *  @param[out] r_thread   The allocated thread ident_t.
 *  @param[out] r_group    The group ident_t that the thread belongs to.
 *  @return  A result code.
 */
result_t kernel_create_thread(ident_t group, const uchar_t *name, size_t name_size,
                          ident_t *r_thread, ident_t *r_group) {
    ident_t thread;

    if (group == 0) {
        group = create_thread_group();
    } else {
        BUG_IF(group >= THREAD_NUM_MAX, "invalid group ID");
	BUG_IF(!thread_groups[group].used, "tried to join uncreated group");
    }

    thread = alloc_thread_id();
    threads[thread].group = &thread_groups[group];
    thread_groups[group].num++;

    *r_thread = thread;
    *r_group  = group;

    INFO("created a new thread #%d:%d", group, thread);
    return OK;
}


/**
 *  Resumes the next thread
 */
NORETURN void kernel_resume_next_thread(void) {
    struct thread *t;
    struct thread_group *g;
    ident_t next;

    next = get_next_thread();
    t = &threads[next];
    g = t->group;
    hal_resume_thread(next, &t->hal, &g->vm.hal);
}


/**
 *  Do a soft context switch
 *
 */
void kernel_switch_thread(void) {
    struct thread *t;

    t = &threads[hal_get_current_thread_id()];
    t->status = THREAD_RUNNABLE;

    hal_switch_thread(&t->hal);
}



/**
 *  Do a hard context switch
 *
 */
void kernel_hard_switch_thread(void) {
    struct thread *t;

    if (!started_threading)
        return;

    t = &threads[hal_get_current_thread_id()];
    hal_save_thread(&t->hal);
    t->status = THREAD_RUNNABLE;

    kernel_resume_next_thread();
}


static void run_thread(ident_t group, const char *name, uintptr_t entry, uintptr_t arg) {
    ident_t thread;
    UNUSED ident_t r_group;
    uintptr_t stack;
    size_t stack_size;

    stack_size = 0x4000; // TODO
    INFO("group=%p", group
    );
    stack = (uintptr_t) kernel_allocate_memory(stack_size, MEMORY_ALLOC_NORMAL);
    kernel_create_thread(group, (const uchar_t *) name, strlen(name), &thread, &r_group);
    kernel_set_thread(thread, entry, arg, stack, stack_size);
    kernel_set_thread_status(thread, THREAD_RUNNABLE);
}


NORETURN static void start_threading(void) {

    started_threading = true;
    kernel_resume_next_thread();
}


/**
 *  Initializes the threading system
 */
void kernel_thread_startup(void) {
    ident_t group, thread;

    INFO("initializing the thread system");

    // create the kernel's thread group and a temporary thread
    kernel_create_thread(0, NULL, 0, &thread, &group);
    hal_set_current_thread_id(thread);
    DEBUG("set current thread:#%d.%d", group, thread);

    hal_set_callback(HAL_CALLBACK_TIMER_TICK, kernel_hard_switch_thread);
    hal_set_callback(HAL_CALLBACK_RUN_THREAD, run_thread);
    hal_set_callback(HAL_CALLBACK_START_THREADING, start_threading);
}
