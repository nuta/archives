#ifndef __ARM_RASPI2_H__
#define __ARM_RASPI2_H__

extern "C" {
void arm_irq_handler(struct hal_thread_regs *regs);
}

namespace arm {
namespace raspi2 {

// `i` it the core number (0, 1, ..., or 4)
#define TIMER_CTRL(i)  ((volatile uint32_t *) 0x40000040 + ((i) * 4))
#define ASM  __asm__ __volatile__

void init_timer();
void timer_handler();

} // namesapce raspi2
} // namespace arm

#endif
