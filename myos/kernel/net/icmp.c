#include "kernel.h"
#include "net.h"



/*
*  Function: sendEchoRequest
*
*    エコー要求を送信する
*
*  Parameters:
*
*    to    -  対象となるIPアドレス
*
*  Returns:
*
*    なし
*
*/

void sendEchoRequest (IPAddress to){

 struct EchoRequest  Packet;
 unsigned int        checksum,i;
 u16                 *p;


  Packet.type       = 0x08;
  Packet.code       = 0x00;
  Packet.checksum   = 0x00;
  Packet.id         = 0x1221;
  Packet.sequence   = 1;
  Packet.data       = 0x1234abcd;


  /*  ネットワーク・バイトオーダに変換  */
  he2ne(&Packet.checksum,   sizeof(u16));
  he2ne(&Packet.id,         sizeof(u16));
  he2ne(&Packet.sequence,   sizeof(u16));
  he2ne(&Packet.data,       sizeof(u32));


  /*  チェックサム計算  */
  checksum=0;
  p= (u16 *) &Packet;
  for(i=0; i < 6; i++)
    checksum += p[i];

  checksum = (checksum&0xffff)+(checksum>>16);

  Packet.checksum = (~checksum == 0x0000)? 0xffff:~checksum;

  sendIPPacket(to, IP_PROTOCOL_ICMP, &Packet, 12);

  return;
}




/*
*  Function: sendARPRequest
*
*    ICMPパケットを受信する
*
*  Parameters:
*
*    Request    -  パケット
*    IPSrc      -  送信元のIPアドレス
*
*  Returns:
*
*    なし
*
*/

void receiveICMPPacket (struct EchoRequest *Request, IPAddress IPSrc){

 struct EchoRequest  Packet;
 unsigned int        checksum, i;
 u16                 *p;

  he2ne(&Request->checksum,   sizeof(u16));
  he2ne(&Request->id,         sizeof(u16));
  he2ne(&Request->sequence,   sizeof(u16));
  he2ne(&Request->data,       sizeof(u32));


  Packet.type       = 0x00;
  Packet.code       = 0x00;
  Packet.checksum   = 0x00;
  Packet.id         = 0x1221;
  Packet.sequence   = 1;
  Packet.data       = 0x1234abcd;


  /*  ネットワーク・バイトオーダに変換  */
  he2ne(&Packet.checksum,   sizeof(u16));
  he2ne(&Packet.id,         sizeof(u16));
  he2ne(&Packet.sequence,   sizeof(u16));
  he2ne(&Packet.data,       sizeof(u32));


  /*  チェックサム計算  */
  checksum=0;
  p= (u16 *) &Packet;
  for(i=0; i < 6; i++)
    checksum += p[i];

    checksum = (checksum&0xffff)+(checksum>>16);

    Packet.checksum = (~checksum == 0x0000)? 0xffff:~checksum;

  sendIPPacket(IPSrc, IP_PROTOCOL_ICMP, &Packet, 12);

  return;
}

