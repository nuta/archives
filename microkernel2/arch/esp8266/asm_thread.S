// int arch_yield(struct arch_thread *thread)
.globl arch_yield
arch_yield:
    j 1f
.literal_position
.align 4
1:
        movi    a3, return_to_thread
        s32i.n  a3, a2, 0  // pc
        s32i.n  a1, a2, 4  // sp
        s32i.n  a0, a2, 8 // return address

        // s32i.n  a2, a2 , 12 // argument

        // callee-saved registers
        s32i.n  a12, a2, 16
        s32i.n  a13, a2, 20
        s32i.n  a14, a2, 24
        s32i.n  a15, a2, 28

        movi    a2, 0
        ret.n

return_to_thread:
        movi    a2, 1
        ret.n


// void esp8266_switch_thread(struct arch_thread *thread)
.globl esp8266_switch_thread
esp8266_switch_thread:
        mov.n   a3, a2
        l32i.n  a4, a3, 0 // pc
        l32i.n  a1, a3, 4 // sp
        l32i.n  a0, a3, 8 // return address
        l32i.n  a2, a3, 12 // argument

        // callee-saved registers
        l32i.n  a12, a3, 16
        l32i.n  a13, a3, 20
        l32i.n  a14, a3, 24
        l32i.n  a15, a3, 28

        jx      a4
