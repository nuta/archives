# Thread and Thread Group
A *thread* is an execution unit; each thread has its own CPU state (register) and stack. It is 
equivalent to traditional operating systems. If you are not familiar with thread, try
[a multi-threading tutorial](https://www.toptal.com/python/beginners-guide-to-concurrency-and-parallelism-in-python).

A *thread group* is a set of threads that share resources: memory (strictly, virtual address space),
and channels. In UNIX, *Process* is the same concept.
