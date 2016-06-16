#include "kernel.h"
#include <resea.h>
#include <resea/cpp/memory.h>
#include <resea/kernel.h>
#include <resea/thread.h>
#include <resea/memory.h>
#include <resea/pager.h>
#include <resea/zeroed_pager.h>
#include <resea/channel.h>
#include <resea/datetime.h>
#include <resea/io.h>
#include "handler.h"

namespace kernel {

void server_handler(channel_t __ch, payload_t *m) {
    if ((m[0] & 1) != 1) {
        WARN("the first payload is not inline one (expected inline msgid_t)");
        return;
    }

    switch (EXTRACT_MSGID(m)) {
    case MSGID(pager, fill):
        DEBUG("received pager.fill");
        pager_server::handle_fill(
            __ch
            , (ident_t) EXTRACT(m, pager, fill, id)
            , (offset_t) EXTRACT(m, pager, fill, offset)
            , (size_t) EXTRACT(m, pager, fill, size)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    case MSGID(datetime, get_date):
        DEBUG("received datetime.get_date");
        datetime_server::handle_get_date(
            __ch
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    case MSGID(datetime, set_oneshot_timer):
        DEBUG("received datetime.set_oneshot_timer");
        datetime_server::handle_set_oneshot_timer(
            __ch
            , (channel_t) EXTRACT(m, datetime, set_oneshot_timer, ch)
            , (uintmax_t) EXTRACT(m, datetime, set_oneshot_timer, msec)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    case MSGID(datetime, set_interval_timer):
        DEBUG("received datetime.set_interval_timer");
        datetime_server::handle_set_interval_timer(
            __ch
            , (channel_t) EXTRACT(m, datetime, set_interval_timer, ch)
            , (uintmax_t) EXTRACT(m, datetime, set_interval_timer, msec)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    case MSGID(datetime, delay):
        DEBUG("received datetime.delay");
        datetime_server::handle_delay(
            __ch
            , (uintmax_t) EXTRACT(m, datetime, delay, msec)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    case MSGID(io, allocate):
        DEBUG("received io.allocate");
        io_server::handle_allocate(
            __ch
            , (resea::interfaces::io::space_t) EXTRACT(m, io, allocate, iospace)
            , (uintptr_t) EXTRACT(m, io, allocate, addr)
            , (size_t) EXTRACT(m, io, allocate, size)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    case MSGID(io, release):
        DEBUG("received io.release");
        io_server::handle_release(
            __ch
            , (resea::interfaces::io::space_t) EXTRACT(m, io, release, iospace)
            , (uintptr_t) EXTRACT(m, io, release, addr)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    case MSGID(thread, create):
        DEBUG("received thread.create");
        thread_server::handle_create(
            __ch
            , (ident_t) EXTRACT(m, thread, create, group)
            , (uchar_t*) EXTRACT(m, thread, create, name)
            , (size_t) EXTRACT(m, thread, create, name_size)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
        release_memory((void * ) m[__PINDEX(m, thread, create, name)]);
        release_memory((void * ) m[__PINDEX(m, thread, create, name_size)]);
#endif

    case MSGID(thread, delete):
        DEBUG("received thread.delete");
        thread_server::handle_delete(
            __ch
            , (ident_t) EXTRACT(m, thread, delete, thread)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    case MSGID(thread, start):
        DEBUG("received thread.start");
        thread_server::handle_start(
            __ch
            , (ident_t) EXTRACT(m, thread, start, thread)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    case MSGID(thread, set):
        DEBUG("received thread.set");
        thread_server::handle_set(
            __ch
            , (ident_t) EXTRACT(m, thread, set, thread)
            , (uintptr_t) EXTRACT(m, thread, set, entry)
            , (uintptr_t) EXTRACT(m, thread, set, arg)
            , (uintptr_t) EXTRACT(m, thread, set, stack)
            , (size_t) EXTRACT(m, thread, set, stack_size)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    case MSGID(thread, get_current_thread):
        DEBUG("received thread.get_current_thread");
        thread_server::handle_get_current_thread(
            __ch
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    case MSGID(memory, map):
        DEBUG("received memory.map");
        memory_server::handle_map(
            __ch
            , (ident_t) EXTRACT(m, memory, map, group)
            , (uintptr_t) EXTRACT(m, memory, map, addr)
            , (size_t) EXTRACT(m, memory, map, size)
            , (channel_t) EXTRACT(m, memory, map, pager)
            , (ident_t) EXTRACT(m, memory, map, pager_arg)
            , (offset_t) EXTRACT(m, memory, map, offset)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    case MSGID(memory, unmap):
        DEBUG("received memory.unmap");
        memory_server::handle_unmap(
            __ch
            , (uintptr_t) EXTRACT(m, memory, unmap, addr)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    case MSGID(memory, get_page_size):
        DEBUG("received memory.get_page_size");
        memory_server::handle_get_page_size(
            __ch
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    case MSGID(memory, allocate):
        DEBUG("received memory.allocate");
        memory_server::handle_allocate(
            __ch
            , (size_t) EXTRACT(m, memory, allocate, size)
            , (uint32_t) EXTRACT(m, memory, allocate, flags)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    case MSGID(memory, release):
        DEBUG("received memory.release");
        memory_server::handle_release(
            __ch
            , (uintptr_t) EXTRACT(m, memory, release, addr)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    case MSGID(memory, allocate_physical):
        DEBUG("received memory.allocate_physical");
        memory_server::handle_allocate_physical(
            __ch
            , (paddr_t) EXTRACT(m, memory, allocate_physical, paddr)
            , (size_t) EXTRACT(m, memory, allocate_physical, size)
            , (uint32_t) EXTRACT(m, memory, allocate_physical, flags)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    case MSGID(channel, connect):
        DEBUG("received channel.connect");
        channel_server::handle_connect(
            __ch
            , (uintmax_t) EXTRACT(m, channel, connect, channel)
            , (interface_t) EXTRACT(m, channel, connect, interface)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    case MSGID(channel, register):
        DEBUG("received channel.register");
        channel_server::handle_register(
            __ch
            , (uintmax_t) EXTRACT(m, channel, register, channel)
            , (interface_t) EXTRACT(m, channel, register, interface)
        );
        return;

#ifndef KERNEL
        // free readonly payloads sent via kernel (user-space)
#endif

    }

    WARN("unsupported message: msgid=%#x", EXTRACT_MSGID(m));
}

} // namespace kernel