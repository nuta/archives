interfaces += timer interrupt
objs += \
    kernel/init.o       \
    kernel/halt.o       \
    kernel/event.o      \
    kernel/list.o       \
    kernel/interrupt.o  \
    kernel/process.o    \
    kernel/thread.o     \
    kernel/resources.o  \
    kernel/timer.o      \
    kernel/kmalloc.o    \
    kernel/open.o       \
    kernel/send.o       \
    kernel/recv.o       \
    kernel/call.o       \
    kernel/discard.o    \
    kernel/transfer.o   \
    kernel/link.o       \
    kernel/channel.o
