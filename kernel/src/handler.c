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
        WARN("the first payload is not inline one (service)");
        return;
    }

    switch (payloads[1]) {
    case SERVICE(pager, fill):
    {
        DEBUG("received pager.fill");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            kernel_pager_fill(__ch, (ident_t) a0, (offset_t) a1, (size_t) a2);
            return;
    }
    case SERVICE(datetime, get_date):
    {
        DEBUG("received datetime.get_date");
            kernel_datetime_get_date(__ch);
            return;
    }
    case SERVICE(datetime, set_oneshot_timer):
    {
        DEBUG("received datetime.set_oneshot_timer");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            kernel_datetime_set_oneshot_timer(__ch, (channel_t) a0, (uintmax_t) a1);
            return;
    }
    case SERVICE(datetime, set_interval_timer):
    {
        DEBUG("received datetime.set_interval_timer");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            kernel_datetime_set_interval_timer(__ch, (channel_t) a0, (uintmax_t) a1);
            return;
    }
    case SERVICE(datetime, delay):
    {
        DEBUG("received datetime.delay");
            payload_t a0 = payloads[2];
            kernel_datetime_delay(__ch, (uintmax_t) a0);
            return;
    }
    case SERVICE(io, read8):
    {
        DEBUG("received io.read8");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            kernel_io_read8(__ch, (io_IOSpace) a0, (uintptr_t) a1, (offset_t) a2);
            return;
    }
    case SERVICE(io, read16):
    {
        DEBUG("received io.read16");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            kernel_io_read16(__ch, (io_IOSpace) a0, (uintptr_t) a1, (offset_t) a2);
            return;
    }
    case SERVICE(io, read32):
    {
        DEBUG("received io.read32");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            kernel_io_read32(__ch, (io_IOSpace) a0, (uintptr_t) a1, (offset_t) a2);
            return;
    }
    case SERVICE(io, read64):
    {
        DEBUG("received io.read64");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            kernel_io_read64(__ch, (io_IOSpace) a0, (uintptr_t) a1, (offset_t) a2);
            return;
    }
    case SERVICE(io, write8):
    {
        DEBUG("received io.write8");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            payload_t a3 = payloads[5];
            kernel_io_write8(__ch, (io_IOSpace) a0, (uintptr_t) a1, (offset_t) a2, (uint8_t) a3);
            return;
    }
    case SERVICE(io, write16):
    {
        DEBUG("received io.write16");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            payload_t a3 = payloads[5];
            kernel_io_write16(__ch, (io_IOSpace) a0, (uintptr_t) a1, (offset_t) a2, (uint16_t) a3);
            return;
    }
    case SERVICE(io, write32):
    {
        DEBUG("received io.write32");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            payload_t a3 = payloads[5];
            kernel_io_write32(__ch, (io_IOSpace) a0, (uintptr_t) a1, (offset_t) a2, (uint32_t) a3);
            return;
    }
    case SERVICE(io, write64):
    {
        DEBUG("received io.write64");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            payload_t a2 = payloads[4];
            payload_t a3 = payloads[5];
            kernel_io_write64(__ch, (io_IOSpace) a0, (uintptr_t) a1, (offset_t) a2, (uint64_t) a3);
            return;
    }
    case SERVICE(thread, create):
    {
        DEBUG("received thread.create");
            payload_t a0 = payloads[2];
            payload_t a1 = payloads[3];
            kernel_thread_create(__ch, (ident_t) a0, (uchar_t*) a1);
            return;
    }
    case SERVICE(thread, delete):
    {
        DEBUG("received thread.delete");
            payload_t a0 = payloads[2];
            kernel_thread_delete(__ch, (ident_t) a0);
            return;
    }
    case SERVICE(thread, start):
    {
        DEBUG("received thread.start");
            payload_t a0 = payloads[2];
            kernel_thread_start(__ch, (ident_t) a0);
            return;
    }
    case SERVICE(thread, block):
    {
        DEBUG("received thread.block");
            payload_t a0 = payloads[2];
            kernel_thread_block(__ch, (ident_t) a0);
            return;
    }
    case SERVICE(thread, unblock):
    {
        DEBUG("received thread.unblock");
            payload_t a0 = payloads[2];
            kernel_thread_unblock(__ch, (ident_t) a0);
            return;
    }
    case SERVICE(thread, set):
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
    case SERVICE(thread, get_current_thread):
    {
        DEBUG("received thread.get_current_thread");
            kernel_thread_get_current_thread(__ch);
            return;
    }
    case SERVICE(memory, map):
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
    case SERVICE(memory, unmap):
    {
        DEBUG("received memory.unmap");
            payload_t a0 = payloads[2];
            kernel_memory_unmap(__ch, (uintptr_t) a0);
            return;
    }
    case SERVICE(memory, get_page_size):
    {
        DEBUG("received memory.get_page_size");
            kernel_memory_get_page_size(__ch);
            return;
    }
    case SERVICE(channel, connect):
    {
        DEBUG("received channel.connect");
            payload_t a0 = payloads[2];
            kernel_channel_connect(__ch, (interface_t) a0);
            return;
    }
    }

    WARN("unsupported service: interface=%d, serivce=%d", payloads[2] >> 16, payloads[1] & 0xffff);
}
