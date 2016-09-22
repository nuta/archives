// int arch_yield(struct arch_thread *thread)
.globl _arch_yield
_arch_yield:
.globl arch_yield
arch_yield:
        leaq    return_to_thread(%rip), %rax
        movq    %rax, 8*0(%rdi)
        movq    %rbx, 8*1(%rdi)
        movq    %rbp, 8*2(%rdi)
        movq    %rdi, 8*3(%rdi) // TODO: it does not make sense
        movq    %rsi, 8*4(%rdi)
        movq    %rsp, 8*5(%rdi)
        movq    %r12, 8*6(%rdi)
        movq    %r13, 8*7(%rdi)
        movq    %r14, 8*8(%rdi)
        movq    %r15, 8*9(%rdi)
        movq    $0, %rax
        retq

return_to_thread:
        movq    $1, %rax
        retq

// void x86_switch_thread(struct arch_thread *thread)
.globl _x86_switch_thread
_x86_switch_thread:
.globl x86_switch_thread
x86_switch_thread:
        movq    %rdi, %rdx
        movq    8*0(%rdx), %rax // RIP
        movq    8*1(%rdx), %rbx
        movq    8*2(%rdx), %rbp
        movq    8*3(%rdx), %rdi
        movq    8*4(%rdx), %rsi
        movq    8*5(%rdx), %rsp
        movq    8*6(%rdx), %r12
        movq    8*7(%rdx), %r13
        movq    8*8(%rdx), %r14
        movq    8*9(%rdx), %r15
        jmpq    *%rax
