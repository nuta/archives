#include "kernel.h"
#include "net.h"



/*
*  Function: sendARPRequest
*
*    ARP要求を送信する
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

void sendARPRequest (IPAddress to){

 struct ARPPacket   Packet;


  // ARP要求
  setMACAddress(&Packet.hwSrc, &Net.MACAddr);
  Packet.IPDest         = to;
  Packet.IPSrc          = Net.IPAddr;
  Packet.hardwareType   = 0x0001;
  Packet.protocolType   = 0x0800;
  Packet.hardwareSize   = 0x06;
  Packet.protocolSize   = 0x04;
  Packet.operation      = 0x0001;


  // ネットワーク・バイトオーダに変換
  he2ne(&Packet.hardwareType,  sizeof(u16));
  he2ne(&Packet.protocolType,  sizeof(u16));
  he2ne(&Packet.operation,     sizeof(u16));
  he2ne(&Packet.IPSrc,         sizeof(u32));
  he2ne(&Packet.IPDest,        sizeof(u32));


  sendFrame(Net.ARP.Cache[0].MACAddr, FRAME_ARP, &Packet, ARP_PACKET_SIZE);
  return;
}




/*
*  Function: reiceiveARPRequest
*
*    ARP要求を受信し、ARP応答を送信する
*
*  Parameters:
*
*    Request    -  ARP要求のパケット
*
*  Returns:
*
*    なし
*
*/

void receiveARPRequest (struct ARPPacket *Request){

 struct ARPPacket Reply;


  // ARP応答
  setMACAddress(&Reply.hwSrc,  &Net.MACAddr);
  setMACAddress(&Reply.hwDest, &Request->hwSrc);

  Reply.IPDest         = Request->IPSrc;
  Reply.IPSrc          = Net.IPAddr;
  Reply.hardwareType   = 0x0001;
  Reply.protocolType   = 0x0800;
  Reply.hardwareSize   = 0x06;
  Reply.protocolSize   = 0x04;
  Reply.operation      = 0x0002;

  // ネットワーク・バイトオーダに変換
  he2ne(&Reply.hardwareType,  sizeof(u16));
  he2ne(&Reply.protocolType,  sizeof(u16));
  he2ne(&Reply.operation,     sizeof(u16));
  he2ne(&Reply.IPSrc,         sizeof(u32));
  he2ne(&Reply.IPDest,        sizeof(u32));

  sendFrame(Request->hwSrc, FRAME_ARP, &Reply, ARP_PACKET_SIZE);

  return;
}




/*
*  Function: receiveARPReply
*
*    ARP応答を受信する
*
*  Parameters:
*
*    Reply    -  ARP応答のパケット
*
*  Returns:
*
*    なし
*
*/

void receiveARPReply (struct ARPPacket *Reply){

 unsigned int i;


  for(i=1; (Net.ARP.Cache[i].IPAddr != Reply->IPSrc) && (i < ARP_CACHE_MAX); i++);

  if(i == ARP_CACHE_MAX)
    return;


  setMACAddress(&Net.ARP.Cache[i].MACAddr, &Reply->hwSrc);
  Net.ARP.Cache[i].status = ARPCACHE_CACHED;

  return;
}




/*
*  Function: receiveARPPacket
*
*    ARPパケットを受信する
*
*  Parameters:
*
*    Packet    -  ARPパケット
*
*  Returns:
*
*    なし
*
*/

void receiveARPPacket (struct ARPPacket *Packet){


  ne2he(&Packet->hardwareType,  sizeof(u16));
  ne2he(&Packet->protocolType,  sizeof(u16));
  ne2he(&Packet->operation,     sizeof(u16));
  ne2he(&Packet->IPSrc,         sizeof(u32));
  ne2he(&Packet->IPDest,        sizeof(u32));


  // ARP要求
  if(Packet->operation==0x0001){

    receiveARPRequest(Packet);
    return;
  }


  // ARP応答
  if(Packet->operation==0x0002){

    receiveARPReply(Packet);
    return;
  }


  // 不明なオペレーション
  return;
}

