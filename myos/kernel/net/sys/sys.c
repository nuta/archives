#include "kernel.h"
#include "net.h"

void asm_intNE2000Handler (void);
void NE2000_init (u8 *MACAddress);
void NE2000_send (void *frame, u32 size);


struct NetInfo Net;



/*
*  Function: initNetwork
*
*    ネットワーク機能を初期化する
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

void initNetwork (void){


  Net.IPAddr    = 0;
  Net.gotIPAddr = false;


  // NE2000からの割り込みハンドラを設定
  setIntDescriptor(0x2b, GDT_KERNEL_CODE_SELECTOR, (u64) &asm_intNE2000Handler, IDT_INT_HANDLER);

  // NICドライバを初期化し、ついでにMACアドレスを取得
  NE2000_init((u8 *) &Net.MACAddr);

  print("\nMAC Address is %d:%d:%d:%d:%d:%d\n", Net.MACAddr[0],
                                                Net.MACAddr[1],
                                                Net.MACAddr[2],
                                                Net.MACAddr[3],
                                                Net.MACAddr[4],
                                                Net.MACAddr[5]
       );

  // ブロードキャストアドレス
  Net.ARP.Cache[0].IPAddr     = 0xffffffff;
  Net.ARP.Cache[0].MACAddr[0] = 0xff;
  Net.ARP.Cache[0].MACAddr[1] = 0xff;
  Net.ARP.Cache[0].MACAddr[2] = 0xff;
  Net.ARP.Cache[0].MACAddr[3] = 0xff;
  Net.ARP.Cache[0].MACAddr[4] = 0xff;
  Net.ARP.Cache[0].MACAddr[5] = 0xff;

  //FIXME
 uCount ct;

  ct = createTCPConnection(0, 0, 80);
  Net.TCP.Connection[ct].status = TCP_CONT_CLOSED;

  // IPアドレスを取得
  getIPAddrByDHCP();

  // IPアドレスの取得を待つ
  asm_sti();
  while(!Net.gotIPAddr);
  asm_cli();

  print("IP Address is %d.%d.%d.%d\n\n", (Net.IPAddr>>24) &0xff,
                                         (Net.IPAddr>>16) &0xff,
                                         (Net.IPAddr>>8)  &0xff,
                                         (Net.IPAddr>>0)  &0xff
       );

  for(;;) asm_stihlt();

  return;
}




/*
*  Function: sendPacket
*
*    フレームを送信する
*
*  Parameters:
*
*    frame -  フレーム
*    size  -  サイズ
*
*  Returns:
*
*    なし
*
*/

void sendPacket (void *frame, u32 size){

  NE2000_send(frame, size);
  return;
}

