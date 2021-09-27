#include "kernel.h"



/*
*  Function: exception0
*
*    ゼロ除算例外のハンドラ
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

void exception0 (void){

  print("Exception 0:  Divide Fault");
  halt();
}




/*
*  Function: exception1
*
*    デバッグ例外のハンドラ
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

void exception1 (void){

  print("Exception 1:  Debug");
  halt();
}




/*
*  Function: exception2
*
*    NMI割り込みのハンドラ
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

void exception2 (void){

  print("Exception 2:  NMI Interrupt");
  halt();
}




/*
*  Function: exception3
*
*    ブレークポイント トラップのハンドラ
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

void exception3 (void){

  print("Exception 3:  Breakpoint");
  halt();
}




/*
*  Function: exception4
*
*    オーバーフロー トラップのハンドラ
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

void exception4 (void){

  print("Exception 4:  Overflow");
  halt();
}




/*
*  Function: exception5
*
*    境界チェック例外のハンドラ
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

void exception5 (void){

  print("Exception 5:  Bound Range Exceeded");
  halt();
}




/*
*  Function: exception6
*
*    無効オペコード例外のハンドラ
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

void exception6 (void){

  print("Exception 6:  Invalid Opcode Fault");
  halt();
}




/*
*  Function: exception7
*
*    コプロセッサ利用不可例外のハンドラ
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

void exception7 (void){

  print("Exception 7:  Device Not Avaliable Fault");
  halt();
}




/*
*  Function: exception8
*
*    ダブル フォールト例外のハンドラ
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

void exception8 (void){

  print("Exception 8:  Double Fault");
  halt();
}




/*
*  Function: exception9
*
*    コプロセッサ セグメント オーバーラン例外のハンドラ
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

void exception9 (void){

  print("Exception 9:  CoProcessor Segment Overrun");
  halt();
}




/*
*  Function: exception10
*
*    無効TSS例外のハンドラ
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

void exception10 (void){

  print("Exception 10: Invalid TSS");
  halt();
}




/*
*  Function: exception11
*
*    不在セグメント例外のハンドラ
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

void exception11 (void){

  print("Exception 11: Segment Not Present");
  halt();
}




/*
*  Function: exception12
*
*    スタックセグメント例外のハンドラ
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

void exception12 (void){

  print("Exception 12: Stack Segment Fault");
  halt();
}




/*
*  Function: exception13
*
*    一般保護例外のハンドラ
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

void exception13 (u32 errcode){

  print("Exception 13: General Protection Fault");
  print("\nerrcode: 0x%x", errcode);
  halt();
}




/*
*  Function: exception14
*
*    不在ページ例外のハンドラ
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

void exception14 (u64 errcode){

 u64 cr2;

  cr2 = asm_loadCR2();

  print("\nException 14: Page Fault\n");
  print("task:    %d\n", Task.running);
  print("errcode: 0x%x\n", errcode);
  print("CR2:     0x%x:%x\n", cr2>>32, cr2);

  // ページがない
  if((errcode&1) == 0){

    // スーパバイザモード
    if(((errcode>>2)&1) == 0){


halt();
      setPagingTable((void *) PAGING_TABLE_ADDRESS, cr2, cr2-BASE_ADDRESS, PG_WRITABLE, PG_KERNEL);

    }else{

      print("Exception 14: Page Fault (the processor was executing in user mode)\n");
      halt();
    }

  // 権限がない
  }else{

    print("Exception 14: Page Fault (caused by a page-level protection violation)\n");
    quitTask(Task.running);
  }


  return;
}




/*
*  Function: exception15
*
*    予約された例外のハンドラ (使用していません)
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

void exception15 (void){

  print("Exception 15: ?");
  halt();
}




/*
*  Function: exception16
*
*    コプロセッサ エラー例外のハンドラ
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

void exception16 (void){

  print("Exception 16: Floating-Point Error");
  halt();
}





/*
*  Function: exception17
*
*    アライメント チェック例外のハンドラ
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

void exception17 (void){

  print("Exception 17: Alignment Check Fault");
  halt();
}





/*
*  Function: exception18
*
*    マシン チェック例外のハンドラ
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

void exception18 (void){

  print("Exception 18: Machine Check");
  halt();
}





/*
*  Function: exception19
*
*    SIMD 浮動小数点例外のハンドラ
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

void exception19 (void){

  print("Exception 19: SIMD Floating-Point");
  halt();
}

