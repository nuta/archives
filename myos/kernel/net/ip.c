#include "kernel.h"
#include "std.h"
#include "net.h"



/*
*  Function: sendIPPacket
*
*    IPパケットを送信する
*
*  Parameters:
*
*    IPDest    -  送信先のIPアドレス
*    protocol  -  上位層のプロトコル
*    data      -  データ
*    size      -  データのサイズ
*
*  Returns:
*
*    なし
*
*/

void sendIPPacket (IPAddress IPDest, enum IP_PROTOCOL_TYPE protocol, 
             void *data, unsigned int size){

 struct IPPacket       Packet;
 MACAddress           *hwDest;
 u16                   id;
 unsigned int          i;
 unsigned int          checksum;
 u16                  *p;

  // MACアドレスを取得

  MACAddress defaultGateway;
  hwDest = &defaultGateway;
  defaultGateway[0] = 0x52;
  defaultGateway[1] = 0x55;
  defaultGateway[2] = 0x0a;
  defaultGateway[3] = 0x00;
  defaultGateway[4] = 0x02;
  defaultGateway[5] = 0x03;

//  hwDest = getMACAddress(IPDest);


  // id
  Net.IP.id++;
  id = Net.IP.id;

  if(id == IP_ID_MAX){
    id           = 0;
    Net.IP.id  = 0;
  }


  // IPパケットを作成
  Packet.version_headerLength = 0x45;
  Packet.tos                  = 0x00;
  Packet.dataLength           = 20+size;
  Packet.id                   = id;
  Packet.flag_fragOffset      = 0x00;
  Packet.ttl                  = IP_PACKET_TTL;
  Packet.protocol             = protocol;
  Packet.checksum             = 0x0000;
  Packet.IPSrc                = Net.IPAddr;
  Packet.IPDest               = IPDest;
  memcpy(&Packet.data, data, size);

  //ネットワーク・バイトオーダに変換
  he2ne(&Packet.dataLength,        sizeof(u16));
  he2ne(&Packet.id,                sizeof(u16));
  he2ne(&Packet.flag_fragOffset,   sizeof(u16));
  he2ne(&Packet.checksum,          sizeof(u16));
  he2ne(&Packet.IPSrc,             sizeof(u32));
  he2ne(&Packet.IPDest,            sizeof(u32));



  // チェックサム計算
  checksum = 0;

  p= (u16 *) &Packet;

  for(i=0; i < 10; i++)
    checksum += p[i];


  checksum = (checksum&0xffff)+(checksum>>16);

  Packet.checksum = (~checksum == 0x0000)? 0xffff:~checksum;


  sendFrame(*hwDest, FRAME_IP, &Packet, IP_HEADER_LENGTH+size);

  return;
}




/*
*  Function: receiveIPPacket
*
*    IPパケットを受信する
*
*  Parameters:
*
*    Packet -  IPパケット
*
*  Returns:
*
*    なし
*
*/

void receiveIPPacket (struct IPPacket *Packet){


  ne2he(&Packet->dataLength,       sizeof(u16));
  ne2he(&Packet->id,               sizeof(u16));
  ne2he(&Packet->flag_fragOffset,  sizeof(u16));
  ne2he(&Packet->checksum,         sizeof(u16));
  ne2he(&Packet->IPSrc,            sizeof(u32));
  ne2he(&Packet->IPDest,           sizeof(u32));

  // 自分宛でなかったらパケットを捨てる
  if((Packet->IPDest != Net.IPAddr) && (Packet->IPDest != IP_BROADCAST_ADDRESS))
    return;

  if(Packet->protocol == IP_PROTOCOL_TCP)
    receiveTCPPacket((struct TCPPacket *) &Packet->data, Packet->IPSrc, Packet->dataLength-IP_HEADER_LENGTH);

  if(Packet->protocol == IP_PROTOCOL_UDP)
    receiveUDPPacket((struct UDPPacket *) &Packet->data, Packet->IPSrc, Packet->dataLength-IP_HEADER_LENGTH);

  return;
}

