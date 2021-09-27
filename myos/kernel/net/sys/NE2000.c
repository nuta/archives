#include "kernel.h"
#include "net.h"


#define NE2000_PORT         0xc000
#define NE2000_CR           NE2000_PORT+0
#define NE2000_PSTART       NE2000_PORT+1
#define NE2000_PSTOP        NE2000_PORT+2
#define NE2000_BNRY         NE2000_PORT+3
#define NE2000_TPSR         NE2000_PORT+4
#define NE2000_TBCR0        NE2000_PORT+5
#define NE2000_TBCR1        NE2000_PORT+6
#define NE2000_ISR          NE2000_PORT+7
#define NE2000_CURR         NE2000_PORT+7 /* (!) Page:1 */
#define NE2000_RSAR0        NE2000_PORT+8
#define NE2000_RSAR1        NE2000_PORT+9
#define NE2000_RBCR0        NE2000_PORT+10
#define NE2000_RBCR1        NE2000_PORT+11
#define NE2000_RCR          NE2000_PORT+12
#define NE2000_RSR          NE2000_PORT+12
#define NE2000_TCR          NE2000_PORT+13
#define NE2000_DCR          NE2000_PORT+14
#define NE2000_IMR          NE2000_PORT+15
#define NE2000_DATA_PORT    NE2000_PORT+16




void NE2000_init (u8 *MACAddress){

 int i;

  /*  各レジスタ初期化  */
  asm_out8(NE2000_CR,        0x21);
  asm_out8(NE2000_DCR,       0x5c);
  asm_out8(NE2000_RBCR0,     0x00);
  asm_out8(NE2000_RBCR1,     0x00);
  asm_out8(NE2000_IMR,       0xff);
  asm_out8(NE2000_ISR,       0xff);
  asm_out8(NE2000_TCR,       0x02);
  asm_out8(NE2000_RCR,       0x20);
  asm_out8(NE2000_PSTART,    0x64);
  asm_out8(NE2000_PSTOP,     0x80);
  asm_out8(NE2000_BNRY,      0x46);
  asm_out8(NE2000_CR,        0x61);
  asm_out8(NE2000_CURR,      0x47);
  asm_out8(NE2000_CR,        0x21);


  /*  MACアドレス取得  */
  asm_out8(NE2000_RSAR0,     0x00);
  asm_out8(NE2000_RSAR1,     0x00);
  asm_out8(NE2000_RBCR0,     12);
  asm_out8(NE2000_RBCR1,     0x00);
  asm_out8(NE2000_CR,        0x0a);

  for(i=0; i < 6; i++){
    MACAddress[i] = asm_in8(NE2000_DATA_PORT);
    asm_in8(NE2000_DATA_PORT);   /* 読み捨てる */
  }

  /*  再設定  */
  asm_out8(NE2000_TCR,       0x00);
  asm_out8(NE2000_RCR,       0x1c);
  asm_out8(NE2000_DCR,       0x58);

  return;
}




void NE2000_writeBuffer (void *dataP, u16 size, u16 page){

 int i;
 u8 *data = (u8 *) dataP;



  /*  ISRのRDCをクリア  */
  asm_out8(NE2000_ISR,   asm_in8(NE2000_ISR)&0xbf);

  /*  データのバイト数をRBCRに書き込む  */
  asm_out8(NE2000_RBCR0, size      & 0xff);
  asm_out8(NE2000_RBCR1, (size>>8) & 0xff);

  /*  転送先のアドレスをRSARに書き込む  */
  asm_out8(NE2000_RSAR0, page      & 0xff);
  asm_out8(NE2000_RSAR1, (page>>8) & 0xff);

  /*  CRのRD1をセット  */
  asm_out8(NE2000_CR,    0x12);

  /* データ送信 */
  for(i=0; i < size; i++){
    asm_out8(NE2000_DATA_PORT, data[i]);
  }

  return;
}




void NE2000_readBuffer  (void *dataP, u16 size, u16 page){

 int i;
 u8 *data = (u8 *) dataP;



  /*  データのバイト数をRBCRに書き込む  */
  asm_out8(NE2000_RBCR0, size      & 0xff);
  asm_out8(NE2000_RBCR1, (size>>8) & 0xff);

  /*  RSARにページを書き込む  */
  asm_out8(NE2000_RSAR0, page      & 0xff);
  asm_out8(NE2000_RSAR1, (page>>8) & 0xff);

  /*  CRのRD0をセット  */
  asm_out8(NE2000_CR,     0x0a);

  /*  データ取得  */
  for(i=0; i < size; i++)
    data[i] = asm_in8(NE2000_DATA_PORT);

  return;
}




void NE2000_send (void *frame, u32 size){


  /*  送信中なら待つ  */
  while((asm_in8(NE2000_CR))&0x04);

  /*  バッファに書き込む  */
  NE2000_writeBuffer(frame, size, 0x4000);

  /*  TPSRにバッファに書き込んだデータの先頭アドレスを書き込む  */
  asm_out8(NE2000_TPSR, 0x40);

  /*  TBCRにバッファに書き込んだデータのサイズを書き込む*/
  asm_out8(NE2000_TBCR0, size      & 0xff);
  asm_out8(NE2000_TBCR1, (size>>8) & 0xff);

  /*  送信  */
  asm_out8(NE2000_CR, asm_in8(NE2000_CR)|0x04);

  return;
}



struct NE2000_RecvInfo{
  unsigned char    status;
  unsigned char    nextPacket;
  unsigned short   length;
  unsigned int     bnry;
  unsigned int     curr;
};



bool NE2000_receive (struct NE2000_RecvInfo *RecvInfo){

 u8   curr,   bnry;
 u8   status;
 u16  length;
 u8   nextPacket;
 u8   buf[256];

  /*  BNRY+1 = CURR の関係になっていないかチェック  */

  asm_out8(NE2000_CR,  0x4a);
  curr = asm_in8(NE2000_CURR);
  asm_out8(NE2000_CR,  0x0a);
  bnry = asm_in8(NE2000_BNRY);


  if(curr == bnry+1)
    return false;


  asm_out8(NE2000_CR,  0x0a);

  /*  BNRY+1のページのデータを4バイト取得する  */
  NE2000_readBuffer(buf, 4, (bnry+1)<<8);

  /*  結果を読み取る  */
  status     = *((u8  *) buf);
  nextPacket = *((u8  *) ((void *) buf+1));
  length     = *((u16 *) ((void *) buf+2));


  if((status&1) != 1)
    return false;

  if((length < 64) || (length > 1518))
    return false;


  /*  BNRYを更新  */
  asm_out8(NE2000_BNRY, curr-1);

  /*  割り込みが終了したことを通知  */
  asm_out8(NE2000_ISR,  0xff);

  /*  受信の結果を作成  */
  RecvInfo->nextPacket = nextPacket;
  RecvInfo->status     = status;
  RecvInfo->length     = length;
  RecvInfo->curr       = curr;
  RecvInfo->bnry       = bnry+1;

  return true;
}






void intNE2000Handler (void){

 char buf[1700];
 unsigned int  i;
 struct NE2000_RecvInfo RecvInfo;


  while(NE2000_receive(&RecvInfo)){

    for(i=RecvInfo.bnry; (i < RecvInfo.nextPacket) && (i < RecvInfo.curr); i++)
      NE2000_readBuffer((void *) &buf+((i-RecvInfo.bnry)*256), 256, ((i)<<8)+4);

    receiveFrame(&buf);
  }

  asm_out8(NE2000_ISR,  0xff);
  return;
}

