#ifndef __X86_ASM_H__
#define __X86_ASM_H__

#include "common.h"

#define C_SYMBOL(symbol)  symbol /* symbols used from C */
.intel_syntax noprefix
.text

/*
 *  macros to save/restore registers
 */
.macro SAVE_REGS
  push  r15
  push  r14
  push  r13
  push  r12
  push  r11
  push  r10
  push  r9
  push  r8
  push  rbp
  push  rdi
  push  rsi
  push  rdx
  push  rcx
  push  rbx
  push  rax
.endm

.macro RESTORE_REGS
  pop   rax
  pop   rbx
  pop   rcx
  pop   rdx
  pop   rsi
  pop   rdi
  pop   rbp
  pop   r8
  pop   r9
  pop   r10
  pop   r11
  pop   r12
  pop   r13
  pop   r14
  pop   r15
.endm

#endif
