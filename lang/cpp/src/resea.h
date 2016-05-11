#ifndef __RESEA_H__
#define __RESEA_H__

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PACKED      __attribute__((packed))
#define NORETURN    __attribute__((noreturn))
#define UNUSED      __attribute__((unused))
#define DEPRECATED  __attribute__((deprecated))

#ifndef __cplusplus

#ifndef NULL
#define NULL  ((void *) 0)
#endif

#endif

typedef uintmax_t payload_t;
typedef uint8_t payloadtype_t;
typedef uintmax_t ident_t;
typedef uintmax_t paddr_t;
typedef uintmax_t offset_t;
typedef payload_t channel_t;
typedef payload_t msgid_t;
typedef msgid_t interface_t;
typedef uint8_t   result_t;
typedef uint8_t uchar_t; /* UTF-8 */
typedef void handler_t (channel_t, payload_t *);

/*
 *  Mutex
 */
typedef uint8_t  mutex_t;
#define MUTEX_LOCKED   (1 << 1)
#define MUTEX_UNLOCKED 0

void init_mutex (mutex_t *m, int init);
void lock_mutex (mutex_t *m);
void unlock_mutex (mutex_t *m);

enum {
  OK             = 1,
  E_NOTFOUND     = 2,
  E_NOTSUPPORTED = 3,
  E_INVALID      = 4,
  E_UNEXPECTED   = 5,
  E_NOSPACE      = 6,
  E_UNCONNECTED  = 7,
  E_INUSE        = 8
};

/*
 *  Logging
 */
void printfmt (const char *format, ...);

#ifndef PACKAGE_NAME
#define PACKAGE_NAME  "somewhere"
#endif

#undef BUG
#define BUG(fmt, ...)    printfmt("[" PACKAGE_NAME "] BUG: "   fmt, ##__VA_ARGS__)
#undef INFO
#define INFO(fmt, ...)   printfmt("[" PACKAGE_NAME "] INFO: "  fmt, ##__VA_ARGS__)
#undef DEBUG
#define DEBUG(fmt, ...)  printfmt("[" PACKAGE_NAME "] DEBUG: " fmt, ##__VA_ARGS__)
#undef MSG
#define MSG(fmt, ...)    printfmt("[" PACKAGE_NAME "] MSG: "   fmt, ##__VA_ARGS__)
#undef WARN
#define WARN(fmt, ...)   printfmt("[" PACKAGE_NAME "] WARN: "  fmt, ##__VA_ARGS__)


void hal_panic(void);

#define BUG_IF(cond, fmt, ...)  do { \
  if (cond) \
      printfmt("[" PACKAGE_NAME "] BUG: "   fmt, ##__VA_ARGS__); \
  } while (0)

#define PANIC(fmt, ...) do{ \
      printfmt("[%s:%s:%d] PANIC: " fmt, __FILE__, __func__, __LINE__, \
                ##__VA_ARGS__); \
      hal_panic(); \
  }while(0)

/*
 *  Test
 */
#define TEST_EXPECT_DESC(desc, cond) printfmt("[" PACKAGE_NAME "] TEST: <%s> " desc,  (cond)? "pass":"fail")
#define TEST_EXPECT(cond)            printfmt("[" PACKAGE_NAME "] TEST: <%s> " #cond, (cond)? "pass":"fail")
#define TEST_END() do { \
                printfmt("[" PACKAGE_NAME "] TEST: end"); \
                hal_panic(); \
            } while(0)

/*
 *  Useful macros
 */
#define ALIGN(x, align) (((x) + (align) - 1) & ~((align) - 1))

/*
 *  System calls
 */
enum {
  SYSCALL_OPEN       = 1,
  SYSCALL_CLOSE      = 2,
  SYSCALL_SETOPTIONS = 3,
  SYSCALL_GETOPTIONS = 4,
  SYSCALL_SEND       = 5,
  SYSCALL_RECV       = 6,
  SYSCALL_WAIT       = 7,
  SYSCALL_CALL       = 8,
  SYSCALL_LINK       = 9,
  SYSCALL_TRANSFER   = 10,
};

channel_t sys_open(void);
result_t sys_close(channel_t ch);
result_t sys_wait(channel_t ch);
result_t sys_setoptions(channel_t id, handler_t handler, void *buffer, size_t size);
result_t sys_send(channel_t ch, payload_t *m, size_t size);
payload_t *sys_recv(channel_t channel);
result_t sys_call(channel_t ch, payload_t *m, size_t size, void *buffer, size_t buffer_size);
result_t sys_link(channel_t ch1, channel_t ch2);
result_t sys_transfer(channel_t ch1, channel_t ch2);


/*
 *  Channel
 */
channel_t create_channel();
void close_channel(channel_t channel);
void set_channel_handler(channel_t channel, handler_t handler);
channel_t connect_to_local(channel_t id);
NORETURN void serve_channel(channel_t ch, handler_t handler);


/*
 *  Message
 */
#define INTERFACE(i)         __interface_##i
#define MSGID(i, type)       __msgid_##i##_##type
#define __PTYPE(i, t, p)     __ptype_##i##_##t##_##p
#define __PINDEX(i, t, p)    __pindex_##i##_##t##_##p
#define EXTRACT(m, i, t, p)  ((__PTYPE(i, t, p)) (((payload_t *) (m))[__PINDEX(i, t, p)]))
#define EXTRACT_MSGID(m)     ((m)[1])

enum {
  PAYLOAD_NULL     = 0,
  PAYLOAD_INLINE   = 1,
  PAYLOAD_CHANNEL  = 2,
  PAYLOAD_OOL      = 3,
  PAYLOAD_MOVE_OOL = 4,
};


#ifdef __cplusplus
}
#endif
#endif
