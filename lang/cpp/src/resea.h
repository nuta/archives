#ifndef __RESEA_H__
#define __RESEA_H__

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

#define PACKED   __attribute__((packed))
#define NORETURN __attribute__((noreturn))
#define UNUSED   __attribute__((unused))

#ifndef NULL
#define NULL  ((void *) 0)
#endif

typedef uintmax_t payload_t;
typedef uintmax_t ident_t;
typedef uintmax_t paddr_t;
typedef uintmax_t offset_t;
typedef payload_t channel_t;
typedef payload_t service_t;
typedef service_t interface_t;
typedef uint8_t   result_t;
typedef uint8_t uchar_t; /* UTF-8 */
typedef void handler_t ();

/*
 *  mutex_t
 */
typedef uint8_t  mutex_t;
#define MUTEX_LOCKED   (1 << 1)
#define MUTEX_INTERRUPT_ENABLED  (1 << 2)
#define MUTEX_UNLOCKED 0

void init_mutex (mutex_t *m, int init);
void lock_mutex (mutex_t *m);
void unlock_mutex (mutex_t *m);

enum result_tValue{
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
#define MOVE(p) ((void *) (PTR2ADDR(p) | 1))
#define OURS_CHANNEL()
#define THEIRS_CHANNEL()

/*
 *  Logging TODO
 */
void printfmt (const char *format, ...);

#define __TOSTR(x)  #x


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
                for(;;); /* FIXME */ \
            } while(0)

/*
 *  Useful macros
 */
#define INTERFACE(itf)         __interface_##itf
#define SERVICE(itf, service)  __service_##itf##_##service
#define ALIGN(x, align) (((x) + (align) - 1) & ~((align) - 1))

/* system calls */
enum SyscallType{
  SYSCALL_OPEN       = 1,
  SYSCALL_CLOSE      = 2,
  SYSCALL_SETOPTIONS = 3,
  SYSCALL_GETOPTIONS = 4,
  SYSCALL_SEND       = 5,
  SYSCALL_RECV       = 6,
  SYSCALL_WAIT       = 7,
  SYSCALL_CALL       = 8
};


/*
 *  System calls
 */
result_t sys_open(channel_t *ch);
result_t sys_close(channel_t ch);
result_t sys_wait(channel_t ch);
result_t sys_setoptions(channel_t id, handler_t handler, void *buffer, size_t size);
result_t sys_send(channel_t ch, payload_t *m);
result_t sys_recv(channel_t channel, payload_t **m);
result_t sys_call(channel_t ch, payload_t *m, void *buffer, size_t buffer_size);


/*
 *  channel_t
 */
result_t connect_channel(channel_t ch, interface_t interface);
result_t register_channel(channel_t ch, interface_t interface);
NORETURN void serve_channel(channel_t ch, handler_t handler);


/*
 *  IO
 */
#define MEMORY_BARRIER() __asm__ __volatile__("": : :"memory")
#define PTR2ADDR(x) ((uintptr_t) (x))
result_t IORead8(uint8_t iospace, uintmax_t base,
               offset_t offset, uint8_t *data);
result_t IORead16(uint8_t iospace, uintmax_t base,
                offset_t offset, uint16_t *data);
result_t IORead32(uint8_t iospace, uintmax_t base,
                offset_t offset, uint32_t *data);
result_t IOWrite8(uint8_t iospace, uintmax_t base,
               offset_t offset, uint8_t data);
result_t IOWrite16(uint8_t iospace, uintmax_t base,
                offset_t offset, uint16_t data);
result_t IOWrite32(uint8_t iospace, uintmax_t base,
                offset_t offset, uint32_t data);

/*
 * Memory
 */
enum{
  ALLOCMEM_NORMAL       = 0,        /* no requirements */
  ALLOCMEM_PAGE_ALIGNED = (1 << 1), /* aligned to PAGE_SIZE */
  ALLOCMEM_CONTINUOUS   = (1 << 2), /* straight in physical memory space */
  ALLOCMEM_ZEROED       = (1 << 3), /* zero-filled */
  ALLOCMEM_USER         = (1 << 4), /* map in user virtual memory, used by kernel */
};

void *allocMemory(size_t size, uint32_t flags);
void freeMemory(void *p);
void *allocPhysicalMemory(paddr_t addr, size_t size, uint32_t flags, paddr_t *alloced_addr); // TODO: remove

#endif
