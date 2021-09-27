#include "kernel.h"
#include "std.h"
#include "net.h"

/*
*  Function: sendUDPPacket
*
*    UDPパケットを送信する
*
*  Parameters:
*
*    portSrc   -  送信元のポート
*    portDest  -  送信先のポート
*    IPDest    -  送信先のIPアドレス
*    data      -  データ
*    size      -  データのサイズ
*
*  Returns:
*
*    なし
*
*/

void sendUDPPacket (IPAddress IPDest, Port portSrc, Port portDest, void *data, int size){

 struct UDPPacket Packet;

  Packet.portSrc    = portSrc;
  Packet.portDest   = portDest;
  Packet.dataLength = UDP_HEADER_LENGTH+size;
  Packet.checksum   = 0x0000;
  memcpy(&Packet.data, data, size);


  // ネットワーク・バイトオーダに変換
  he2ne(&Packet.portSrc,     sizeof(u16));
  he2ne(&Packet.portDest,    sizeof(u16));
  he2ne(&Packet.dataLength,  sizeof(u16));
  he2ne(&Packet.checksum,    sizeof(u16));


  sendIPPacket(IPDest, IP_PROTOCOL_UDP, &Packet, UDP_HEADER_LENGTH+size);

  return;
}




/*
*  Function: receiveUDPPacket
*
*    UDPパケットを受信する
*
*  Parameters:
*
*    Packet  -  UDPパケット
*    IPSrc   -  送信元のIPアドレス
*    size    -  パケットのサイズ
*
*  Returns:
*
*    なし
*
*/

void receiveUDPPacket (struct UDPPacket *Packet, IPAddress IPSrc, u16 size){



  ne2he(&Packet->portSrc,     sizeof(u16));
  ne2he(&Packet->portDest,    sizeof(u16));
  ne2he(&Packet->dataLength,  sizeof(u16));
  ne2he(&Packet->checksum,    sizeof(u16));

  //
  //  DHCPのみを処理
  //

  if(Packet->portDest == 68)
    receiveDHCPPacket((struct DHCPPacket *) &Packet->data);

  return;
}

