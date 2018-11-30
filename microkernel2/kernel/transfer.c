#include "channel.h"
#include "message.h"


result_t _transfer(struct channel *from, struct channel *to) {
    // TODO: add a assertion

    from->transfer_to = to;
    return OK;
}


result_t transfer(cid_t from, cid_t to) {

    struct channel *_from = get_channel_by_cid(from);
    struct channel *_to   = get_channel_by_cid(to);

    if (!_from || !_to) {
        return E_BAD_CID;
    }

    return _transfer(_from, _to);
}
