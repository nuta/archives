Virutal Memory
==============

Virtual Address Space Layout
----------------------------

```
      0       +-------------------+
              |                   |  The beginning area is not used
              |    never used     |  to detect NULL pointer dereferences.
              |                   |
  (variable)  +-------------------+  The beginning address and length are specified in
              |    user program   |  the executable file.
              |      (.text)      |
              |-------------------|
              |     user data     |
              |      (.data)      |
   BOX_BASE   |-------------------|  Box is the region for stuffs to be allocated dynamically:
              |                   |  messages payloads, thread stacks, memory block allocated
              |  message payloads |  by `memory.allocate`, for example.
              |         .         |
              |         .         |
              |         .         |
              |                   |
              |-------------------|
              |  thread #1 info   |  Thread info page exists at the bottom of
              |-------------------|  the thread's stack:
              |  thread #1 stack  |
              |-------------------|     thread_info = stack_pointer & ~(stack_size - 1)
              |  thread #2 info   |
              |-------------------|
              |  thread #2 stack  |
              |-------------------|
              |         .         |
              |         .         |
              |         .         |
 KERNEL_BASE  |-------------------|
              |                   |  Kernel space maps from physical address 0. (straight mapping)
              |   kernel space    |  All pages are wired; page-out does not occurs.
              |                   |
              +-------------------+

```

