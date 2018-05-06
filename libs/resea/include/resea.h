#ifndef __RESEA_H__
#define __RESEA_H__

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
typedef unsigned char u8_t;
typedef unsigned short u16_t;
typedef unsigned u32_t;
typedef unsigned long long u64_t;
typedef unsigned char bool;
typedef char * string_t;
typedef u8_t * buffer_t;

#define NULL ((void *) 0)
#define false 0
#define true  1
#define offsetof __builtin_offsetof
#define STATIC_ASSERT _Static_assert
#define UNUSED __attribute__((unused))
#define PACKED __attribute__((packed))
#define NORETURN __attribute__((noreturn))
#define DEPRECATED __attribute__((deprecated))
#define UNREACHABLE __builtin_unreachable()
#define va_list __builtin_va_list
#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_end(ap) __builtin_va_end(ap)
#define va_arg(ap, type) __builtin_va_arg(ap, type)

int printf(const char *fmt, ...);

#include <arch.h>

void *memcpy(void *dest, const void *src, size_t size);
void *memset(void *p, int c, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
size_t strlen(const char *s);

struct list {
    struct list *next;
};

#define DEFINE_LIST(name, type)                                     \
    static inline void name##_list_init(type **list) {              \
        list_init((struct list **) list);                           \
    }                                                               \
                                                                    \
    static inline bool name##_list_is_empty(type **list) {          \
        return *list == NULL;                                       \
    }                                                               \
                                                                    \
    static inline void name##_list_append(type **list, type *e) {   \
        list_append((struct list **) list, e);                      \
    }                                                               \
                                                                    \
    static inline type *name##_list_pop(type **list) {              \
        return list_pop((struct list **) list);                     \
    }                                                               \
                                                                    \
    static inline void name##_list_remove(type **list, type *e) {   \
        list_remove((struct list **) list, e);                      \
    }

void list_init(struct list **list);
void list_append(struct list **list, void *e);
void *list_pop(struct list **list);
void list_remove(struct list **list, void *e);

static inline bool is_computer_on(void) {
    return true;
}


static inline double is_computer_on_fire(void) {
    return 2.7182818284;
}

#define TYPES_OFFSET   0ULL
#define ERROR_OFFSET    24ULL
#define MINOR_ID_OFFSET 32ULL
#define MAJOR_ID_OFFSET 40ULL
#define MSGTYPE(header) ((header) >> MINOR_ID_OFFSET)
#define ERRTYPE(header) (((header) >> MAJOR_ID_OFFSET) & 0xff)

enum {
    ERROR_NONE = 0,
    ERROR_NO_MEMORY = 1,
    ERROR_INVALID_CH = 2,
    ERROR_CH_NOT_LINKED = 3,
    ERROR_CH_NOT_TRANSFERED = 4,
    ERROR_CH_IN_USE = 5,
};

typedef header_t error_t;

// TODO: interface generator
#define SERVICE_ID(major, minor) (((major) << MAJOR_ID_OFFSET) | ((minor) << MINOR_ID_OFFSET))

#define SYSCALL_OPEN 1
#define SYSCALL_CLOSE 2
#define SYSCALL_SEND 3
#define SYSCALL_RECV 4
#define SYSCALL_CALL 5
#define SYSCALL_REPLYRECV 6
#define SYSCALL_LINK 7
#define SYSCALL_TRANSFER 8
#define SYSCALL_CONNECT 9

channel_t ipc_open(void);
header_t ipc_send(
    channel_t ch,
    header_t type,
    payload_t a0,
    payload_t a1,
    payload_t a2,
    payload_t a3
);

header_t ipc_recv(
    channel_t ch,
    channel_t *from,
    payload_t *a0,
    payload_t *a1,
    payload_t *a2,
    payload_t *a3
);

header_t ipc_call(
    channel_t ch,
    header_t type,
    payload_t a0,
    payload_t a1,
    payload_t a2,
    payload_t a3,
    payload_t *r0,
    payload_t *r1,
    payload_t *r2,
    payload_t *r3
);

header_t ipc_replyrecv(
    channel_t *client,
    header_t type,
    payload_t r0,
    payload_t r1,
    payload_t r2,
    payload_t r3,
    payload_t *a0,
    payload_t *a1,
    payload_t *a2,
    payload_t *a3
);

header_t ipc_discard(
    payload_t ool0,
    payload_t ool1,
    payload_t ool2,
    payload_t ool3
);

header_t ipc_link(channel_t ch0, channel_t ch1);
header_t ipc_transfer(channel_t ch, channel_t dest);
channel_t ipc_connect(channel_t server);

#endif
