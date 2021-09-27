#include "kernel.h"



/*
*  Function: initTask
*
*    ページングテーブルを切り替える
*
*  Parameters:
*
*    Pgtbl -  ページテーブル
*
*  Returns:
*
*    なし
*
*/

void changePagingTable (struct PML4 *Pgtbl){

 address addr;

  addr = (u64) Pgtbl;
  asm_setCR3(addr&0xffffffff);

  return;
}




/*
*  Function: setPagingTable
*
*    ページエントリを作成・変更する
*
*  Parameters:
*
*    Pgtbl -  ページテーブル
*    pAddr -  物理アドレス
*    vAddr -  仮想アドレス
*    rw    -  書き込みを許可するか
*    us    -  特権レベル
*
*  Returns:
*
*    なし
*
*/

void setPagingTable (struct PML4 *Pml4, address pAddr, address  vAddr,
                     enum PG_RW rw,     enum PG_US us){

 uIndex    i,j,k,l;
 address   addr;
 u64      *p;


 //
 //  PML4E,PDPTE...の場所を取得
 //

  i = (vAddr>>39)&0x00000000000001ff;
  j = (vAddr>>30)&0x00000000000001ff;
  k = (vAddr>>21)&0x00000000000001ff;
  l = (vAddr>>12)&0x00000000000001ff;



 //
 //  PML4E,PDPTE...を書き込む
 //

  //PML4

  p = &Pml4->entry[i];

  //PML4エントリが存在しない場合は、作成する
  if((*p&1) == 1){

    addr = *p&0x000000fffffff000;

  }else{

    addr = (u64) kmalloc(PDPT_SIZE);
    *p   = (addr&0xfffffff000)+(PG_USER<<2)+(PG_WRITABLE<<1)+1;
  }


  if(addr < BASE_ADDRESS)
    addr += BASE_ADDRESS;



  //PDPT

  p = ((void *) addr+(j*(PDPTE_SIZE)));


  //PDPTエントリが存在しない場合は、作成する
  if((*p&1) == 1){

    addr = *p&0x000000fffffff000;

  }else{

    addr = (u64) kmalloc(PDT_SIZE);
    *p   = addr+(PG_USER<<2)+(PG_WRITABLE<<1)+1;
  }


  if(addr < BASE_ADDRESS)
    addr += BASE_ADDRESS;


  //PDT

  p = ((void *) addr+(k*(PDTE_SIZE)));

  //PDTエントリが存在しない場合は、作成する
  if((*p&1) == 1){

    addr = *p&0x000000fffffff000;

  }else{

    addr = (u64) kmalloc(PDT_SIZE);
    *p   = addr+(PG_USER<<2)+(PG_WRITABLE<<1)+1;
  }

  if(addr < BASE_ADDRESS)
    addr += BASE_ADDRESS;

  //PT

  p = ((void *) addr+(l*(PTE_SIZE)));
  *p   = pAddr+(us<<2)+(rw<<1)+1;

  return;

}

