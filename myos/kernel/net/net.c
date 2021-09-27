#include "kernel.h"
#include "std.h"
#include "net.h"



/*
*  Function: sendFrame
*
*    フレームを送信する
*
*  Parameters:
*
*    Frame -  フレーム
*
*  Returns:
*
*    なし
*
*/

void sendFrame (MACAddress hwDest, enum FRAME_TYPE type, void *data, u32 size){

 struct MacFrame Frame;


  // フレーム
  setMACAddress(&Frame.hwDest, &hwDest);
  setMACAddress(&Frame.hwSrc,  &Net.MACAddr);
  Frame.type = type;

  memcpy(&Frame.data, data, size);


  // ネットワーク・バイトオーダに変換
  he2ne(&Frame.type, sizeof(u16));

  sendPacket(&Frame, size+FRAME_HEADER_LENGTH);
  return;
}




/*
*  Function: receiveFrame
*
*    フレームを受信する
*
*  Parameters:
*
*    frame -  フレーム
*
*  Returns:
*
*    なし
*
*/

void receiveFrame (void *frame){

 u16 type;


  type = *((u16 *) ((void *)  frame+12));
  he2ne(&type, sizeof(u16));

  // ARP
  if(type == FRAME_ARP)
    receiveARPPacket((void *) frame+14);


  //  IP
  if(type == FRAME_IP)
    receiveIPPacket((void *)  frame+14);


  return;
}

