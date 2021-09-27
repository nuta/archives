#include "kernel.h"



/*
*  Function: halt
*
*    システムを停止する
*
*  Parameters:
*
*    なし
*
*  Returns:
*
*    なし
*
*/

void halt (void){

  asm_cli();
  print("\nSystem halted.\n");
  asm_hlt();
}

