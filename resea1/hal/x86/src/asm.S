#include "x86_asm.h"
.text
.code64

.global C_SYMBOL(x86_asm_out8)
C_SYMBOL(x86_asm_out8):
  mov  dx, di
  mov  al, sil

  out  dx, al
  ret

.global C_SYMBOL(x86_asm_in8)
C_SYMBOL(x86_asm_in8):
  mov  dx, di

  in   al, dx
  ret

.global C_SYMBOL(x86_asm_out16)
C_SYMBOL(x86_asm_out16):
  mov  dx, di
  mov  ax, si

  out  dx, ax
  ret

.global C_SYMBOL(x86_asm_in16)
C_SYMBOL(x86_asm_in16):
  mov  dx, di

  in   ax, dx
  ret

.global C_SYMBOL(x86_asm_out32)
C_SYMBOL(x86_asm_out32):
  mov  dx,  di
  mov  eax, esi

  out  dx, eax
  ret

.global C_SYMBOL(x86_asm_in32)
C_SYMBOL(x86_asm_in32):
  mov  dx, di

  in   eax, dx
  ret


//TODO use INVLPG
.globl C_SYMBOL(x86_asm_invalidate_tlb)
C_SYMBOL(x86_asm_invalidate_tlb):
  mov  rax, rdi

  mov  rax, cr3
  mov  cr3, rax
  ret


.globl C_SYMBOL(x86_asm_lgdt)
C_SYMBOL(x86_asm_lgdt):
  mov  rax, rdi

  lgdt [rax]
  ret


.globl C_SYMBOL(x86_asm_lidt)
C_SYMBOL(x86_asm_lidt):
  mov  rax, rdi

  lidt [rax]
  ret


.globl C_SYMBOL(x86_asm_ltr)
C_SYMBOL(x86_asm_ltr):
  mov  ax, di

  ltr  ax
  ret


.globl C_SYMBOL(x86_asm_rdmsr)
C_SYMBOL(x86_asm_rdmsr):
  push rbx
  mov  ecx, edi // the MSR to be read

  xor  rbx, rbx
  rdmsr

  /* upper 32 bits */
  shl  rdx, 32
  and  rax, rdx

  /* lower 32 bits */
  or   rax, rbx
  pop  rbx
  ret


.globl C_SYMBOL(x86_asm_wrmsr)
C_SYMBOL(x86_asm_wrmsr):
  mov  ecx, edi // the MSR to be written
  mov  eax, esi // lower 32 bits data
  shr  rsi, 32
  mov  edx, esi // upper 32 bits data

  wrmsr
  ret


.globl C_SYMBOL(x86_asm_read_cr2)
C_SYMBOL(x86_asm_read_cr2):
  mov  rax, cr2
  ret


.globl C_SYMBOL(x86_asm_interrupt_enabled)
C_SYMBOL(x86_asm_interrupt_enabled):
  /* (rflags >> 9) & 1 */
  pushfq
  pop  rax
  shr  rax, 9
  and  rax, 1
  ret


.globl C_SYMBOL(x86_asm_cli)
C_SYMBOL(x86_asm_cli):
  cli
  ret


.globl C_SYMBOL(x86_asm_sti)
C_SYMBOL(x86_asm_sti):
  sti
  ret


.globl C_SYMBOL(x86_asm_hlt)
C_SYMBOL(x86_asm_hlt):
  hlt
  ret


.globl C_SYMBOL(x86_asm_stihlt)
C_SYMBOL(x86_asm_stihlt):
  sti
  hlt
  ret


.globl C_SYMBOL(x86_asm_resume_thread)
C_SYMBOL(x86_asm_resume_thread):
  cmp  rdi, 1 // RDI == 1 if the thread is in kernel mode
  jne  1f
  mov  ax, 3
  jmp  2f
1:
  mov  ax, 3
2:
  mov  ds, ax
  mov  es, ax
  mov  fs, ax
  mov  gs, ax

  mov  rsp, rsi
  RESTORE_REGS
  iretq

.globl C_SYMBOL(x86_asm_init_syscall)
C_SYMBOL(x86_asm_init_syscall):
  // rdi: the entry point
  // si:  CS for kernel mode
  // dx:  CS for user mode
  push rsi
  push rdi

  mov  rsi, (24 << 48) | (8 << 32) // FIXME: use defined macros not magic numbers
  mov  edi, 0xc0000081
  call C_SYMBOL(x86_asm_wrmsr)

  pop  rsi  // RDI
  mov  edi, 0xc0000082
  call C_SYMBOL(x86_asm_wrmsr)

  mov  rsi, 0
  mov  edi, 0xc0000083
  call C_SYMBOL(x86_asm_wrmsr)

  mov  rsi, 0
  mov  edi, 0xc0000084
  call C_SYMBOL(x86_asm_wrmsr)

  /* enable the SYSCALL/SYSRET instructions */
  mov  edi, 0xc0000080
  call C_SYMBOL(x86_asm_rdmsr)

  mov  rsi, rax
  or   rsi, 1
  mov  edi, 0xc0000080
  call C_SYMBOL(x86_asm_wrmsr)

  pop  rsi
  ret


/* soft context switch */
.globl C_SYMBOL(hal_switch_thread)
C_SYMBOL(hal_switch_thread):
  mov  r8,  rdi
  mov  r9,  rsi
  mov  r10, rbx
  mov  r11, rbp

 /* These registers MUST be saved due to
    the x64 calling convention */
  mov  rsi, rsp  // RSP
  mov  rdx, r8   // RDI
  mov  rcx, r9   // RSI
  mov  r8,  r10  // RBX
  mov  r9,  r11  // RBP
  push r12       // R12
  push r13       // R13
  push r14       // R14
  push r15       // R15

  call x86_soft_save_thread
  call x86_resume_next_thread // this does not return


.global C_SYMBOL(x86_unblocked_thread_entry)
C_SYMBOL(x86_unblocked_thread_entry):
  // unblocked threads comes here
  ret

.global C_SYMBOL(x86_asm_set_cr3)
C_SYMBOL(x86_asm_set_cr3):
  mov  cr3, rdi
  ret


.global C_SYMBOL(x86_asm_fxsave)
C_SYMBOL(x86_asm_fxsave):
  fxsave64 [rdi]
  ret


.global C_SYMBOL(x86_asm_fxrstor)
C_SYMBOL(x86_asm_fxrstor):
  fxrstor64 [rdi]
  ret
