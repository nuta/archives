#ifndef __X64_MSR_H__
#define __X64_MSR_H__

// This file could be included in a assembly file.

#define MSR_APIC_BASE       0x0000001b
#define MSR_GS_BASE         0xc0000101
#define MSR_KERNEL_GS_BASE  0xc0000102
#define MSR_EFER            0xc0000080
#define MSR_STAR            0xc0000081
#define MSR_LSTAR           0xc0000082
#define MSR_CSTAR           0xc0000083
#define MSR_SFMASK          0xc0000084

#define EFER_SCE 0x01

#endif
