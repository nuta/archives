#ifndef __KERNEL_MESSAGE_H__
#define __KERNEL_MESSAGE_H__

#include <types.h>
#include "process.h"

#define PAYLOAD_NULL     0
#define PAYLOAD_INLINE   1
#define PAYLOAD_CHANNEL  2
#define PAYLOAD_POINTER  3
#define PAYLOAD_SIZE     4

#define GET_PAYLOAD_TYPE(header, i) (((header) >> (4 * ((i - 1) % 8))) & 0x0f)
#define IS_PAYLOAD_HEADER_INDEX(i) ((i) % 8 == 0)

struct channel *_get_channel_by_cid(struct process *proc, cid_t cid);
struct channel *get_channel_by_cid(cid_t cid);

#ifndef __RESEA_H__
result_t call(cid_t cid, const void *m, size_t m_size,
              void *r, size_t r_size);
result_t send(cid_t cid, const void *m, size_t size, int flags);
result_t recv(cid_t cid, void *buffer, size_t size, int flags, cid_t *from);
cid_t open(void);
result_t link(cid_t cid1, cid_t cid2);
result_t transfer(cid_t from, cid_t to);
#endif

cid_t _open(struct process *proc);
result_t _link(struct channel *ch1, struct channel *ch2);
result_t _transfer(struct channel *from, struct channel *to);

#endif
