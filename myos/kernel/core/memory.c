#include "kernel.h"


/*
*  Function: isActiveMemoryAddress
*
*    存在する物理アドレス(メインメモリの容量内)かチェック
*
*  Parameters:
*
*    addr -  物理アドレス
*
*  Returns:
*
*    なし
*
*/

bool isActiveMemoryAddress (address addr){

 volatile u64  *p;

  setPagingTable((void *) PAGING_TABLE_ADDRESS, addr, BASE_ADDRESS+addr, PG_WRITABLE, PG_KERNEL);

  p  = (void *) 0xc0000000000+addr;
  *p = 0x55aa55aa55aa55aa;

  return ((*p == 0)? false:true);
}




/*
*  Function: kmalloc
*
*    物理アドレスを割り当てる
*
*  Parameters:
*
*    memsize -  サイズ (バイト単位)
*
*  Returns:
*
*    なし
*
*/

//NOTE: 手抜き
u64 kmalloced = 0x300000;

void *kmalloc (size memsize){

  kmalloced += memsize+0x1000;

  return (void *)  ((kmalloced-(memsize+0x1000)+BASE_ADDRESS)&0xfffffffff000);
}

