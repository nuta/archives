#ifndef __RESEA_X64_H__
#define __RESEA_X64_H__

#include <config.h>
#include <resea/string.h>

static inline struct thread_info *get_thread_info(void) {
    struct thread_info *info;
#ifdef CONFIG_X86_FSGSBASE
    __asm__ __volatile__("rdgsbase %0" : "=a"(info));
#else
    __asm__ __volatile__("mov %%gs:(0), %0" : "=a"(info));
#endif
    return info;
}

static inline void copy_message(struct message *dst,
                                const struct message *src) {
    uint32_t header = src->header;
    // TODO: Use memcpy.
    dst->header = header;
    dst->from = src->from;
    dst->notification = src->notification;
    dst->payloads.channel = src->payloads.channel;
    dst->payloads.page = src->payloads.page;

    memcpy(&dst->payloads.data, &src->payloads.data,
           INLINE_PAYLOAD_LEN(header));
}

static inline void copy_to_ipc_buffer(const struct message *m) {
    copy_message(get_ipc_buffer(), m);
}

static inline void copy_from_ipc_buffer(struct message *buf) {
    copy_message(buf, get_ipc_buffer());
}

static inline error_t sys_ipc(cid_t ch, uint32_t ops) {
    error_t err;
    __asm__ __volatile__(
        "syscall"
        : "=a"(err)
        : "a"(SYSCALL_IPC | ops), "D"(ch)
        : "%rsi", "%rdx", "%rcx", "%r8", "%r9", "%r10", "%r11", "memory");
    return err;
}

static inline int sys_open(void) {
    int cid_or_err;
    __asm__ __volatile__(
        "syscall"
        : "=a"(cid_or_err)
        : "a"(SYSCALL_OPEN)
        : "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9", "%r10", "%r11");
    return cid_or_err;
}

static inline error_t sys_close(cid_t ch) {
    int cid_or_err;
    __asm__ __volatile__(
        "syscall"
        : "=a"(cid_or_err)
        : "a"(SYSCALL_CLOSE), "D"(ch)
        : "%rsi", "%rdx", "%rcx", "%r8", "%r9", "%r10", "%r11");
    return cid_or_err;
}

static inline error_t sys_link(cid_t ch1, cid_t ch2) {
    error_t err;
    __asm__ __volatile__("syscall"
                         : "=a"(err)
                         : "a"(SYSCALL_LINK), "D"(ch1), "S"(ch2)
                         : "%rdx", "%rcx", "%r8", "%r9", "%r10", "%r11");
    return err;
}

static inline error_t sys_transfer(cid_t src, cid_t dst) {
    error_t err;
    __asm__ __volatile__("syscall"
                         : "=a"(err)
                         : "a"(SYSCALL_TRANSFER), "D"(src), "S"(dst)
                         : "%rdx", "%rcx", "%r8", "%r9", "%r10", "%r11");
    return err;
}

static inline void sys_nop(void) {
    error_t err;
    __asm__ __volatile__("syscall"
                         : "=a"(err)
                         : "a"(SYSCALL_NOP)
                         : "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9", "%r10",
                           "%r11");
}

#endif
