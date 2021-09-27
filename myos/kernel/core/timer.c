#include "kernel.h"

#define  IOPORT_TIMER_CTRL    0x43
#define  IOPORT_TIMER_COUNT0  0x40


struct st_Timer{
  uCount  count;
};

struct st_Timer  Timer;


void initTimer (void){

  Timer.count = 0;
  asm_out8(IOPORT_TIMER_CTRL,    0x34);
  asm_out8(IOPORT_TIMER_COUNT0,  TIMER_INTERVAL&0xff);
  asm_out8(IOPORT_TIMER_COUNT0,  TIMER_INTERVAL>>8);

  return;
}



void intTimerHandler (void){

  Timer.count++;
  struct st_Registers *Regs;


  if(Timer.count%TASK_SWITCH_INTERVAL == 0){

    if(Timer.count == UCOUNT_MAX)
      Timer.count = 1;


    Regs = (struct st_Registers *)  REGS_SAVE_ADDRESS;
    saveTask(Task.running, Regs);
    switchTask();

  }


  if(Timer.count == UCOUNT_MAX)
    Timer.count = 1;

  return;
}

