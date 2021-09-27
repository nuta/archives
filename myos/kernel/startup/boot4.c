#include "kernel.h"
#include "std.h"




/*
*  Function: boot4
*
*    エントリーポイント
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

void boot4 (void){

 struct st_GDTR  *GDTR  = (struct st_GDTR *)  GDTR_ADDRESS;
 struct st_IDTR  *IDTR  = (struct st_IDTR *)  IDTR_ADDRESS;
 struct st_TSS   *TSS   = (struct st_TSS  *)  TSS_ADDRESS;
 uCount i;


  //==========================================================================
  //  Screenの初期化
  //==========================================================================


    Screen.VRAMAddress = VRAM_ADDRESS;
    Screen.x           = 0;
    Screen.y           = 0;
    Screen.xMax        = SCREEN_X_MAX;
    Screen.yMax        = SCREEN_Y_MAX;


  //==========================================================================
  //  ウェルカムメッセージ
  //==========================================================================

    print(WELCOME_MSG);



  //==========================================================================
  //  コンピュータの情報を取得
  //==========================================================================


   u32 eax, ebx, ecx, edx;
   char vendorName[13];
   char processorName[47];

    // プロセッサのベンダ名

    asm_cpuid(0, &eax, &ebx, &ecx, &edx);

    vendorName[0]   = (ebx>>0)  &0xff;
    vendorName[1]   = (ebx>>8)  &0xff;
    vendorName[2]   = (ebx>>16) &0xff;
    vendorName[3]   = (ebx>>24) &0xff;
    vendorName[4]   = (edx>>0)  &0xff;
    vendorName[5]   = (edx>>8)  &0xff;
    vendorName[6]   = (edx>>16) &0xff;
    vendorName[7]   = (edx>>24) &0xff;
    vendorName[8]   = (ecx>>0)  &0xff;
    vendorName[9]   = (ecx>>8)  &0xff;
    vendorName[10]  = (ecx>>16) &0xff;
    vendorName[11]  = (ecx>>24) &0xff;
    vendorName[12]  = '\0';



    //プロセッサ名

    //拡張CPUID命令に対応しているかチェック
    if(eax >= 0x80000000){

      for(i=1; i < 4; i++){

        asm_cpuid(0x80000000+i, &eax, &ebx, &ecx, &edx);

        processorName[((i-1)*16)+0]   = (eax>>0)  &0xff;
        processorName[((i-1)*16)+1]   = (eax>>8)  &0xff;
        processorName[((i-1)*16)+2]   = (eax>>16) &0xff;
        processorName[((i-1)*16)+3]   = (eax>>24) &0xff;
        processorName[((i-1)*16)+4]   = (ebx>>0)  &0xff;
        processorName[((i-1)*16)+5]   = (ebx>>8)  &0xff;
        processorName[((i-1)*16)+6]   = (ebx>>16) &0xff;
        processorName[((i-1)*16)+7]   = (ebx>>24) &0xff;
        processorName[((i-1)*16)+8]   = (ecx>>0)  &0xff;
        processorName[((i-1)*16)+9]   = (ecx>>8)  &0xff;
        processorName[((i-1)*16)+10]  = (ecx>>16) &0xff;
        processorName[((i-1)*16)+11]  = (ecx>>24) &0xff;
        processorName[((i-1)*16)+12]  = (edx>>0)  &0xff;
        processorName[((i-1)*16)+13]  = (edx>>8)  &0xff;
        processorName[((i-1)*16)+14]  = (edx>>16) &0xff;
        processorName[((i-1)*16)+15]  = (edx>>24) &0xff;
      }

      processorName[46]  = '\0';

    }else{

      strcpy(processorName, "Unknown");
    }


    print("Processor: %s (%s)\n", processorName, vendorName);



  //==========================================================================
  //  タイマの初期化
  //==========================================================================

    print("setting up timer...\n");

    initTimer();



  //==========================================================================
  //  GDTの設定
  //==========================================================================

    print("setting up GDT...\n");


    setSegDescriptor(GDT_KERNEL_CODE_SELECTOR, 0x00000000, 0xffffffff, GDT_KERNEL_CODESEG);
    setSegDescriptor(GDT_KERNEL_DATA_SELECTOR, 0x00000000, 0xffffffff, GDT_USER_DATASEG);
    setSegDescriptor(GDT_USER_CODE_SELECTOR,   0x00000000, 0xffffffff, GDT_USER_CODESEG);
    setSegDescriptor(GDT_USER_DATA_SELECTOR,   0x00000000, 0xffffffff, GDT_USER_DATASEG);
    setTSSDescriptor(GDT_TSS_SELECTOR,         TSS_ADDRESS, GDT_TSS);


    GDTR->length   = GDT_LENGTH;
    GDTR->address  = GDT_ADDRESS;

    asm_lgdt(GDTR_ADDRESS);



  //==========================================================================
  //  IDTの設定
  //==========================================================================

    print("setting up IDT...\n");

    // 例外ハンドラ
    setIntDescriptor(0,  GDT_KERNEL_CODE_SELECTOR, (u64) &asm_exception0,  IDT_INT_HANDLER);
    setIntDescriptor(1,  GDT_KERNEL_CODE_SELECTOR, (u64) &asm_exception1,  IDT_INT_HANDLER);
    setIntDescriptor(2,  GDT_KERNEL_CODE_SELECTOR, (u64) &asm_exception2,  IDT_INT_HANDLER);
    setIntDescriptor(3,  GDT_KERNEL_CODE_SELECTOR, (u64) &asm_exception3,  IDT_INT_HANDLER);
    setIntDescriptor(4,  GDT_KERNEL_CODE_SELECTOR, (u64) &asm_exception4,  IDT_INT_HANDLER);
    setIntDescriptor(5,  GDT_KERNEL_CODE_SELECTOR, (u64) &asm_exception5,  IDT_INT_HANDLER);
    setIntDescriptor(6,  GDT_KERNEL_CODE_SELECTOR, (u64) &asm_exception6,  IDT_INT_HANDLER);
    setIntDescriptor(7,  GDT_KERNEL_CODE_SELECTOR, (u64) &asm_exception7,  IDT_INT_HANDLER);
    setIntDescriptor(8,  GDT_KERNEL_CODE_SELECTOR, (u64) &asm_exception8,  IDT_INT_HANDLER);
    setIntDescriptor(9,  GDT_KERNEL_CODE_SELECTOR, (u64) &asm_exception9,  IDT_INT_HANDLER);
    setIntDescriptor(10, GDT_KERNEL_CODE_SELECTOR, (u64) &asm_exception10, IDT_INT_HANDLER);
    setIntDescriptor(11, GDT_KERNEL_CODE_SELECTOR, (u64) &asm_exception11, IDT_INT_HANDLER);
    setIntDescriptor(12, GDT_KERNEL_CODE_SELECTOR, (u64) &asm_exception12, IDT_INT_HANDLER);
    setIntDescriptor(13, GDT_KERNEL_CODE_SELECTOR, (u64) &asm_exception13, IDT_INT_HANDLER);
    setIntDescriptor(14, GDT_KERNEL_CODE_SELECTOR, (u64) &asm_exception14, IDT_INT_HANDLER);
    setIntDescriptor(15, GDT_KERNEL_CODE_SELECTOR, (u64) &asm_exception15, IDT_INT_HANDLER);
    setIntDescriptor(16, GDT_KERNEL_CODE_SELECTOR, (u64) &asm_exception16, IDT_INT_HANDLER);
    setIntDescriptor(17, GDT_KERNEL_CODE_SELECTOR, (u64) &asm_exception17, IDT_INT_HANDLER);
    setIntDescriptor(18, GDT_KERNEL_CODE_SELECTOR, (u64) &asm_exception18, IDT_INT_HANDLER);
    setIntDescriptor(19, GDT_KERNEL_CODE_SELECTOR, (u64) &asm_exception19, IDT_INT_HANDLER);

    //ハードウェア割り込みハンドラ
    setIntDescriptor(IDT_TIMER_SELECTOR,    GDT_KERNEL_CODE_SELECTOR, (u64) &asm_intTimerHandler,    IDT_INT_HANDLER);

    IDTR->length   = IDT_LENGTH;
    IDTR->address  = IDT_ADDRESS;

    asm_lidt(IDTR_ADDRESS);




  //==========================================================================
  //  ファイルシステム初期化
  //==========================================================================

    print("setting up File System...\n");
    initFS();




  //==========================================================================
  //  TSS初期化
  //==========================================================================

    print("setting up TSS...\n");


    TSS->rsp0      = TSS_RSP0;
    TSS->ist[1]    = TSS_IST1;
    TSS->ioMapBase = ((u64) &TSS->ioPermMap) - ((u64) TSS);

    for(i=0; i < 8192; i++)
      TSS->ioPermMap[i] = 0;

    asm_ltr(GDT_TSS_SELECTOR);



  //==========================================================================
  //  割り込みの受付を開始
  //==========================================================================


    asm_out8(IOPORT_INTMASK_MASTER, INTERRUPT_MASK_MASTER);
    asm_out8(IOPORT_INTMASK_SLAVE,  INTERRUPT_MASK_SLAVE);



  //==========================================================================
  //  ネットワーク機能を初期化
  //==========================================================================

    print("setting up Networking...\n");
    initNetwork();


  //==========================================================================
  //  API初期化
  //==========================================================================

    print("setting up API...\n");
    initAPI();



  //==========================================================================
  //  タスク管理初期化
  //==========================================================================

    print("setting up Task Management...\n");

    initTask();



  //==========================================================================
  //  シェル起動
  //==========================================================================

    print("starting shell...\n");

    if(fileExists("test.bin") == false)
      print("Cannot not open \"test.bin\".");


    Task.active = 1;
    //startTask("test.bin");

    asm_sti();
    for(;;)
      asm_stihlt();
}

