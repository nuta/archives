#include <resea.h>
#include <mutex.h>
#include "channel.h"
#include "message.h"


result_t call(cid_t cid, const void *m, size_t m_size,
              void *r, size_t r_size) {

    result_t result;

    struct channel *ch = get_channel_by_cid(cid);
    if (!ch) {
        return E_BAD_CID;
    }

    // Lock the receiver lock of `src` channel to prevent
    // other threads from receiving the reply message of this
    // message.
    mutex_lock(&ch->receiver_lock);

    result = _send(ch, m, m_size, 0);
    if (result != OK) {
        return result;
    }

    cid_t from; // We don't care this.
    return _recv(ch, r, r_size, 0, &from);
}
