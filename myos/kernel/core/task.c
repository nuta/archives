#include "kernel.h"



/*
*  Function: initTask
*
*    タスク管理システムの初期化
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

void initTask (void){

 uCount i;

  for(i=0; (i < TASK_ENTRY_MAX); i++)
    Task.Entry[i].status = TASK_UNUSED;

  Task.running      = 0;
  Task.active       = 0;
  Task.runningNum   = 0;

 return;
}



/*
*  Function: quitTask
*
*    タスクを停止し、削除する
*
*  Parameters:
*
*    i -  タスクのID
*
*  Returns:
*
*    なし
*
*/

void quitTask (uIndex i){


  Task.Entry[i].status = TASK_UNUSED;
  Task.runningNum--;
  Task.running = 0;

  return;
}




/*
*  Function: newTask
*
*    新しいタスクを作成する
*
*  Parameters:
*
*    rip -  RIPレジスタ
*    cr3 -  CR3レジスタ
*    rsp -  RSPレジスタ
*
*  Returns:
*
*    成功した場合はtrue、失敗した場合はfalseを返します。
*
*/

bool newTask (u64 rip, u64 cr3, u64 rsp){

 uIndex i;

  for(i=1; (Task.Entry[i].status != TASK_UNUSED) && (i < TASK_ENTRY_MAX); i++);

  if(i == TASK_ENTRY_MAX)
    return false;

  Task.Entry[i].status    = TASK_RUNNING;
  Task.Entry[i].Regs.rip  = rip;
  Task.Entry[i].Regs.cr3  = cr3;
  Task.Entry[i].Regs.rsp  = rsp;
  Task.runningNum++;

  return true;
}




/*
*  Function: startTask
*
*    新しくアプリケーションを起動する
*
*  Parameters:
*
*    filename -  ファイル名
*
*  Returns:
*
*    なし
*
*/


void startTask (const char *filename){

 u64 codeSize;
 struct PML4  *Pgtbl = (struct PML4 *) kmalloc(sizeof(struct PML4));
 void         *stack = (void *)        kmalloc(TASK_STACK_SIZE);
 void         *code  = (void *)        kmalloc(getFilesize(filename));
 uCount i;

  //ファイルが存在するかチェック
  if(fileExists(filename) == false)
    return;

  //ファイルを読み込む
  loadFile(code, filename, getFilesize(filename));

  codeSize = getFilesize(filename);


  // ユーザ
  for(i=0; i < (codeSize/PAGE_SIZE)+1; i++){
    setPagingTable(Pgtbl, (u64) (code+(i*PAGE_SIZE)), (u64) (i*PAGE_SIZE), PG_WRITABLE, PG_USER);
  }

  // スタック
  for(i=0; i < (TASK_STACK_SIZE/PAGE_SIZE); i++){
    setPagingTable(Pgtbl, (u64) (stack+(i*PAGE_SIZE)), (u64) (USER_STACK_ADDRESS+(i*PAGE_SIZE)), PG_WRITABLE, PG_USER);
  }

  // カーネル
  for(i=0; i < ((KERNEL_ADDRESS_END-BASE_ADDRESS)/PAGE_SIZE); i++){
    setPagingTable(Pgtbl, (u64) (i*PAGE_SIZE), (u64) (BASE_ADDRESS+(i*PAGE_SIZE)), PG_WRITABLE, PG_KERNEL);
  }

  newTask(0, (u64) Pgtbl, USER_STACK_ADDRESS+TASK_STACK_SIZE-8);

  return;
}




/*
*  Function: saveTask
*
*    タスクの状態を保存する
*
*  Parameters:
*
*    i    - タスクのID
*    Regs - 各レジスタへのポインタ
*
*
*  Returns:
*K
*    なし
*
*/

void saveTask (uIndex i, struct st_Registers *Regs){



  Task.Entry[i].Regs.rip = Regs->rip;
  Task.Entry[i].Regs.cr3 = Regs->cr3;
  Task.Entry[i].Regs.rax = Regs->rax;
  Task.Entry[i].Regs.rbx = Regs->rbx;
  Task.Entry[i].Regs.rcx = Regs->rcx;
  Task.Entry[i].Regs.rdx = Regs->rdx;
  Task.Entry[i].Regs.rdi = Regs->rdi;
  Task.Entry[i].Regs.rsi = Regs->rsi;
  Task.Entry[i].Regs.rbp = Regs->rbp;
  Task.Entry[i].Regs.rsp = Regs->rsp;
  Task.Entry[i].Regs.r8  = Regs->r8;
  Task.Entry[i].Regs.r9  = Regs->r9;
  Task.Entry[i].Regs.r10 = Regs->r10;
  Task.Entry[i].Regs.r11 = Regs->r11;
  Task.Entry[i].Regs.r12 = Regs->r12;
  Task.Entry[i].Regs.r13 = Regs->r13;
  Task.Entry[i].Regs.r14 = Regs->r14;
  Task.Entry[i].Regs.r15 = Regs->r15;

  return;
}




/*
*  Function: switchTask
*
*    タスクスイッチングをする
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

void switchTask (void){

 uIndex i;

  if((Task.runningNum == 1) && (Task.running != 0))
    return;

  for(i=Task.running+1; (Task.Entry[i].status != TASK_RUNNING); i++){

    if(i == Task.running+1)
      return;

    if(i == TASK_ENTRY_MAX)
      i=0;
  }


  Task.running = i;

  changePagingTable((void *) Task.Entry[i].Regs.cr3-BASE_ADDRESS);
  asm_switchTask(&Task.Entry[i].Regs);
}

