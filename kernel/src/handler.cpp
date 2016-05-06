#include <resea.h>
#include <resea/kernel.h>
#include <resea/channel.h>
#include <resea/memory.h>
#include <resea/thread.h>
#include <resea/io.h>
#include <resea/datetime.h>
#include <resea/pager.h>
#include "handler.h"


void kernel_handler(channel_t __ch, payload_t *payloads) {
    if ((payloads[0] & 1) != 1) {
        WARN("the first payload is not inline one (expected inline msgtype_t)");
        return;
    }

    switch (payloads[1]) {
    case MSGTYPE(pager, fill):
    {
        DEBUG("received pager.fill");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            kernel_pager_fill(__ch, (ident_t) a0, (offset_t) a1, (size_t) a2);
            return;
    }
    case MSGTYPE(datetime, get_date):
    {
        DEBUG("received datetime.get_date");
            kernel_datetime_get_date(__ch);
            return;
    }
    case MSGTYPE(datetime, set_oneshot_timer):
    {
        DEBUG("received datetime.set_oneshot_timer");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            kernel_datetime_set_oneshot_timer(__ch, (channel_t) a0, (uintmax_t) a1);
            return;
    }
    case MSGTYPE(datetime, set_interval_timer):
    {
        DEBUG("received datetime.set_interval_timer");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            kernel_datetime_set_interval_timer(__ch, (channel_t) a0, (uintmax_t) a1);
            return;
    }
    case MSGTYPE(datetime, delay):
    {
        DEBUG("received datetime.delay");
            payload_t a0 = payloads[2];
            kernel_datetime_delay(__ch, (uintmax_t) a0);
            return;
    }
    case MSGTYPE(io, allocate):
    {
        DEBUG("received io.allocate");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            kernel_io_allocate(__ch, (io_space_t) a0, (uintptr_t) a1, (size_t) a2);
            return;
    }
    case MSGTYPE(io, release):
    {
        DEBUG("received io.release");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            kernel_io_release(__ch, (io_space_t) a0, (uintptr_t) a1);
            return;
    }
    case MSGTYPE(thread, create):
    {
        DEBUG("received thread.create");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            kernel_thread_create(__ch, (ident_t) a0, (uchar_t*) a1, (size_t) a2);
            return;
    }
    case MSGTYPE(thread, delete):
    {
        DEBUG("received thread.delete");
            payload_t a0 = payloads[2];
            kernel_thread_delete(__ch, (ident_t) a0);
            return;
    }
    case MSGTYPE(thread, start):
    {
        DEBUG("received thread.start");
            payload_t a0 = payloads[2];
            kernel_thread_start(__ch, (ident_t) a0);
            return;
    }
    case MSGTYPE(thread, block):
    {
        DEBUG("received thread.block");
            payload_t a0 = payloads[2];
            kernel_thread_block(__ch, (ident_t) a0);
            return;
    }
    case MSGTYPE(thread, unblock):
    {
        DEBUG("received thread.unblock");
            payload_t a0 = payloads[2];
            kernel_thread_unblock(__ch, (ident_t) a0);
            return;
    }
    case MSGTYPE(thread, set):
    {
        DEBUG("received thread.set");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            payload_t a3 = payloads[5];
            payload_t a4 = payloads[6];
            kernel_thread_set(__ch, (ident_t) a0, (uintptr_t) a1, (uintptr_t) a2, (uintptr_t) a3, (size_t) a4);
            return;
    }
    case MSGTYPE(thread, get_current_thread):
    {
        DEBUG("received thread.get_current_thread");
            kernel_thread_get_current_thread(__ch);
            return;
    }
    case MSGTYPE(memory, map):
    {
        DEBUG("received memory.map");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            payload_t a3 = payloads[5];
            payload_t a4 = payloads[6];
            payload_t a5 = payloads[7];
            kernel_memory_map(__ch, (ident_t) a0, (uintptr_t) a1, (size_t) a2, (channel_t) a3, (ident_t) a4, (offset_t) a5);
            return;
    }
    case MSGTYPE(memory, unmap):
    {
        DEBUG("received memory.unmap");
            payload_t a0 = payloads[2];
            kernel_memory_unmap(__ch, (uintptr_t) a0);
            return;
    }
    case MSGTYPE(memory, get_page_size):
    {
        DEBUG("received memory.get_page_size");
            kernel_memory_get_page_size(__ch);
            return;
    }
    case MSGTYPE(memory, allocate):
    {
        DEBUG("received memory.allocate");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            kernel_memory_allocate(__ch, (size_t) a0, (uint32_t) a1);
            return;
    }
    case MSGTYPE(memory, release):
    {
        DEBUG("received memory.release");
            payload_t a0 = payloads[2];
            kernel_memory_release(__ch, (uintptr_t) a0);
            return;
    }
    case MSGTYPE(memory, allocate_physical):
    {
        DEBUG("received memory.allocate_physical");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            kernel_memory_allocate_physical(__ch, (paddr_t) a0, (size_t) a1, (uint32_t) a2);
            return;
    }
    case MSGTYPE(channel, connect):
    {
        DEBUG("received channel.connect");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            kernel_channel_connect(__ch, (uintmax_t) a0, (interface_t) a1);
            return;
    }
    case MSGTYPE(channel, register):
    {
        DEBUG("received channel.register");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            kernel_channel_register(__ch, (uintmax_t) a0, (interface_t) a1);
            return;
    }
    }

    WARN("unsupported message: interface=%d, type=%d", payloads[2] >> 16, payloads[1] & 0xffff);
}
