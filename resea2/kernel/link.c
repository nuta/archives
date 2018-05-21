#include "channel.h"


static inline void update_channel_state(struct channel *ch, int state) {

    ch->flags = (ch->flags & (~3)) | state;
}


result_t _link(struct channel *ch1, struct channel *ch2) {
    // TODO: add a assertion

    ch1->linked_to = ch2;
    ch2->linked_to = ch1;
    update_channel_state(ch1, CHANNEL_LINKED);
    update_channel_state(ch2, CHANNEL_LINKED);

    return OK;
}


result_t link(cid_t cid1, cid_t cid2) {

    struct channel *ch1 = get_channel_by_cid(cid1);
    struct channel *ch2 = get_channel_by_cid(cid2);

    if (!ch1 || !ch2) {
        return E_BAD_CID;
    }

    return _link(ch1, ch2);
}
