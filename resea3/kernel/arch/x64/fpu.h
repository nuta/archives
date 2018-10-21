#ifndef __X64_FPU_H__
#define __X64_FPU_H__

#define XSTATE_SIZE 4096
void x64_init_fpu(void);

extern u64_t x64_xsave_mask;

#endif
