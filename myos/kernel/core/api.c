#include "kernel.h"



/*
*  Function: initAPI
*
*    APIを初期化する
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

void initAPI (void){

  asm_initSYSCALL((u64) &asm_APIEntry, GDT_KERNEL_CODE_SELECTOR,
                       GDT_USER_CODE_SELECTOR,  TASK_RFLAGS);

  return;
}




/*
*  Function: APIEntry
*
*    APIのエントリーポイント
*
*  Parameters:
*
*    number -  APIのID
*    ...    -  APIの引数
*
*  Returns:
*
*    各APIに依存します。
*
*/



u64 APIEntry (u64 number, u64 arg1, u64 arg2, u64 arg3, u64 arg4, u64 arg5, u64 arg6, u64 arg7){

  print("\nAPI: SYSCALL/SYSRET Instruction test!\n");

  return 0;
}

