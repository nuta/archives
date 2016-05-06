#include <hal.h>
#include <resea.h>
#include <pthread.h>
#include <unistd.h>

void hal_set_current_thread_id(ident_t id) {

}


ident_t hal_get_current_thread_id(void) {

    return 1; // XXX
}


void hal_set_thread(struct hal_thread *t, bool is_kernel,
                    uintptr_t entry, uintptr_t arg, uintptr_t stack, uintptr_t stack_size) {

    pthread_t thread;
    pthread_create(&thread, nullptr, (void *(*)(void *)) entry, (void *) arg);
}


void hal_resume_thread(ident_t id, struct hal_thread *t, struct hal_vm_space *vms) {

    WARN("hal_resume_thread() is not implemented");
    for(;;) {
        sleep(1024);
    }
}


void hal_save_thread(struct hal_thread *t) {
}


void hal_switch_thread(struct hal_thread *t) {

    WARN("hal_switch_thread() is not implemented");
}
