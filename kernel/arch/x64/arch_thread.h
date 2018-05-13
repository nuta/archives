#ifndef __X64_ARCH_THREAD_H__
#define __X64_ARCH_THREAD_H__

#define GS_RIP         0
#define GS_RSP         8
#define GS_RSP0        40
#define GS_KSTACK      48

#ifndef __ASSEMBLER__

// The kernel GS register points to this.
// Don't forget to update offset above!
struct arch_thread {
    u64_t rip;          // 0
    u64_t rsp;          // 8
    u64_t rflags;       // 16
    u64_t is_user;      // 24
    u64_t gs;           // 32
    u64_t rsp0;         // 40: kstack + sizeof(kstack)
    u64_t kstack;       // 48
};

#endif

#endif

