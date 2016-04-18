#ifndef __RESEA_H__
#define __RESEA_H__

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

#define PACKED      __attribute__((packed))
#define NORETURN    __attribute__((noreturn))
#define UNUSED      __attribute__((unused))
#define DEPRECATED  __attribute__((deprecated))

#ifndef NULL
#define NULL  ((void *) 0)
#endif

typedef uintmax_t payload_t;
typedef uintmax_t ident_t;
typedef uintmax_t paddr_t;
typedef uintmax_t offset_t;
typedef payload_t channel_t;
typedef payload_t msgtype_t;
typedef msgtype_t interface_t;
typedef uint8_t   result_t;
typedef uint8_t uchar_t; /* UTF-8 */
typedef void handler_t ();

/*
 *  mutex_t
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

// TODO: decorate these in payload header
#define MOVE(p) ((void *) ((uintptr_t) (p) | 1))

/*
 *  Logging TODO
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
#define INTERFACE(i)      __interface_##i
#define MSGTYPE(i, type)  __msgtype_##i##_##type
#define ALIGN(x, align) (((x) + (align) - 1) & ~((align) - 1))

/* system calls */
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


/*
 *  System calls
 */
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
channel_t connect_to_local(channel_t id);
NORETURN void serve_channel(channel_t ch, handler_t handler);

#endif
