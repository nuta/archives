#include <resea.h>
#include <resea/channel.h>
#include <logging.h>
#include <kernel/thread.h>
#include <kernel/channel.h>
#include <kernel/message.h>


struct server_entry {
    struct channel *ch;
    interface_t interface;
};


#define ENTRIES_NUM 8
static struct server_entry entries [ENTRIES_NUM];
static channel_t server;
static mutex_t entries_lock = MUTEX_INITIALIZER;

static struct channel *lookup(interface_t interface) {

    mutex_lock(&entries_lock);

    for (int i = 0; i < ENTRIES_NUM; i++) {
        if (entries[i].interface == interface) {
            struct channel *ch = entries[i].ch;
            mutex_unlock(&entries_lock);
            return ch;
        }
    }

    mutex_unlock(&entries_lock);
    return NULL;
}


static result_t register_channel(struct channel *ch, interface_t interface) {

    mutex_lock(&entries_lock);

    for (int i = 0; i < ENTRIES_NUM; i++) {
        if (!entries[i].ch) {
            entries[i].ch = ch;
            entries[i].interface = interface;
            mutex_unlock(&entries_lock);
            return OK;
        }
    }

    mutex_unlock(&entries_lock);
    return E_NOMEM;
}


static void mainloop(void) {
    result_t r;
    payload_t buf[8];
    channel_t reply_to;

    r = recv(server, (void *) &buf, sizeof(buf), 0, &reply_to);
    if (r != OK) {
        WARN("channel-server: recv did not return OK");
        return;
    }

    switch (buf[1]) {
    case CHANNEL_CONNECT:
       {
           DEBUG("channel-server: channel.connect");
           struct channel *ch = get_channel_by_cid(buf[2]);
           if (!ch) {
               send_channel_connect_reply(reply_to, E_BAD_CID, 0);
               break;
           }

           interface_t interface = buf[3];

           struct channel *dest = lookup(interface);
           while (!dest) {
               yield(); // FIXME
               dest = lookup(interface);
           }

           cid_t transfer_cid;
           struct channel *transfer_ch;
           transfer_cid = _open(dest->process);
           transfer_ch  = _get_channel_by_cid(dest->process,
                                               transfer_cid);

           _link(ch, transfer_ch);
           _transfer(transfer_ch, dest);

           send_channel_connect_reply(reply_to, OK, 0);
           break;
       }
    case CHANNEL_REGISTER:
        {
            DEBUG("channel-server: channel.register");
            struct channel *ch = get_channel_by_cid(buf[2]);
            if (!ch) {
                send_channel_register_reply(reply_to, E_BAD_CID, 0);
                break;
            }

            interface_t interface = buf[3];
            result_t r = register_channel(ch, interface);
            send_channel_register_reply(reply_to, r, 0);
            break;
        }
    default:
        WARN("channel-server: unknown messages");
    }
}

void channel_server_startup(void) {
    INFO("starting channel server");

    for (int i = 0; i < ENTRIES_NUM; i++) {
        entries[i].ch = NULL;
    }
    
    server = open();
    if (server != 1) {
        WARN("channel-server: failed to open a channel @1");
        return;
    }

    INFO("channel-server: ready");

    for (;;) {
        mainloop();
    }
}
