#include "kernel.h"




/*
*  Function: setTSSDescriptor
*
*    TSSディスクリプタを設定する
*
*  Parameters:
*
*    dsc    -  ディスクリプタの番号
*    base   -  ベースアドレス
*    info   -  ディスクリプタの設定
*
*  Returns:
*
*    なし
*
*/

void setTSSDescriptor (unsigned int dsc, u64 base, u16 info){

 struct TSSDescriptor *descriptor = (struct TSSDescriptor *) ((void *) GDT_ADDRESS+dsc);


  descriptor->limit1    = GDT_TSS_LIMIT&0xffff;
  descriptor->limit2    = (GDT_TSS_LIMIT>>16)&0x0f;
  descriptor->base1     = base&0xffff;
  descriptor->base2     = (base>>16)&0xff;
  descriptor->base3     = (base>>24)&0xff;
  descriptor->base4     = (base>>32)&0xffffffff;
  descriptor->info      = info;
  descriptor->reserved  = 0;

  return;
}




/*
*  Function: setSegDescriptor
*
*    セグメントディスクリプタを設定する
*
*  Parameters:
*
*    dsc    -  ディスクリプタの番号
*    base   -  ベースアドレス
*    limit  -  リミット
*    info   -  ディスクリプタの設定
*
*  Returns:
*
*    なし
*
*/

void setSegDescriptor (unsigned int dsc, u32 base, u32 limit, u16 info){

 struct SegDescriptor *descriptor = (struct SegDescriptor *) ((void *) (GDT_ADDRESS+dsc));


  descriptor->limit1  = limit&0xffff;
  descriptor->limit2  = ((limit>>16)&0x0f)|GDT_LIMIT2;
  descriptor->base1   = base&0xffff;
  descriptor->base2   = (base>>16)&0xff;
  descriptor->base3   = (base>>24)&0xff;
  descriptor->info    = info;

  return;
}




/*
*  Function: setIntDescriptor
*
*    割り込みディスクリプタを設定する
*
*  Parameters:
*
*    dsc    -  ディスクリプタの番号
*    cs     -  割り込みハンドラのコードセグメントディスクリプタ
*    offset -  割り込みハンドラのオフセット
*    info   -  ディスクリプタの設定
*
*  Returns:
*
*    なし
*
*/

void setIntDescriptor (unsigned int dsc, u16 cs, u64 offset, u16 info){

 struct IntDescriptor *descriptor = (struct IntDescriptor *) IDT_ADDRESS+dsc;


  descriptor->offset1   = (u32) offset&0xffff;
  descriptor->offset2   = (u32) (offset>>16)&0xffff;
  descriptor->offset3   = (u32) (offset>>32)&0xffffffff;
  descriptor->cs        = cs;
  descriptor->info      = info;
  descriptor->reserved  = 0;

  return;
}

