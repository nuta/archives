#ifndef __RESEA_H__
#define __RESEA_H__

#include <types.h>
#include <arch.h>
#include <logging.h>

typedef cid_t       channel_t;
typedef uint32_t    interface_t;
typedef interface_t msgid_t;

//
//  Testing
//
#define __TEST_EXPECT2(cond, desc)  printfmt("T%c> " desc,  (cond)? 'P' : 'F')
#define __TEST_EXPECT1(cond)        printfmt("T%c> " #cond, (cond)? 'P' : 'F')

#define _TEST_EXPECT(_0, _1, _2, FUNC, ...) FUNC
#define TEST_EXPECT(...)  _TEST_EXPECT(_0, ##__VA_ARGS__, __TEST_EXPECT2, __TEST_EXPECT1)(__VA_ARGS__)

#define TEST_END() do {        \
        printfmt("TE> All tests finished."); \
        arch_panic(); \
    } while(0)


//
//  Messaging
//

#ifdef __cplusplus
extern "C" {
#endif

channel_t connect_to_local(channel_t to);
channel_t transfer_to(channel_t to);

#ifndef __KERNEL_MESSAGE_H__ // XXX
channel_t open(void);
result_t call(channel_t ch, const void *m, size_t m_size,
              void *r, size_t r_size);
result_t send(channel_t ch, const void *m, size_t size, int flags);
result_t recv(channel_t ch, void *buffer, size_t size, int flags, channel_t *from);
result_t link(channel_t ch1, channel_t ch2);
result_t transfer(cid_t from, cid_t to);
result_t discard(void *buffer, size_t size);
#endif

#ifdef __cplusplus
}
#endif

#endif
