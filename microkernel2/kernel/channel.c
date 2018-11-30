#include "process.h"
#include "channel.h"
#include "panic.h"


struct channel *_get_channel_by_cid(struct process *proc, cid_t cid) {

    for (struct channel *ch = proc->channels; ch; ch = ch->next) {
        if (ch->cid == cid)
            return ch;
    }

    PANIC("bad cid %d",cid);
}


struct channel *get_channel_by_cid(cid_t cid) {

    return _get_channel_by_cid(get_current_thread()->process, cid);
}
