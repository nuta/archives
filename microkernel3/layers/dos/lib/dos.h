#ifndef __DOS_H__
#define __DOS_H__

#include <stdint.h>

void sysfunc(uint64_t *rax, uint64_t *rbx, uint64_t *rcx, uint64_t *rdx);

// DOS INT 21h functions.
#define SYSFUNC_READ_STDIN   0x0100
#define SYSFUNC_WRITE_STDOUT 0x0200
#define SYSFUNC_EX_EXEC      0xf000

#endif
