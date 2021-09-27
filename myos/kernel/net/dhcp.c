#include "kernel.h"
#include "net.h"



/*
*  Function: getIPAddrByDHCP
*
*    DHCPを使用し、IPアドレスを取得する (DHCP DISCOVERを送信する)
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

void getIPAddrByDHCP (void){

  sendDHCPPacket(DHCP_DISCOVER);
  return;
}




/*
*  Function: sendDHCPPacket
*
*    DHCPパケットを送信する
*
*  Parameters:
*
*    type    -  パケットの種類
*
*  Returns:
*
*    なし
*
*/

void sendDHCPPacket (enum DHCP_PACKET_TYPE type){

 struct DHCPPacket packet;


  packet.opcode                   = 0x01;
  packet.hwType                   = 1;
  packet.hwAddrLength             = 6;
  packet.hopCount                 = 0;
  packet.id                       = 0x1234;
  packet.sec                      = 1;
  packet.IPClient                 = 0;
  packet.IPUser                   = 0;
  packet.IPGateway                = 0;
  packet.vendorInfo.magicCode     = 0x63825363;
  packet.vendorInfo.type[0]       = 0x35;
  packet.vendorInfo.type[1]       = 0x01;
  packet.vendorInfo.type[2]       = type;
  packet.vendorInfo.clientID[0]   = 0x3d;
  packet.vendorInfo.clientID[1]   = 0x07;
  packet.vendorInfo.clientID[2]   = 0x01;

  if(type == DHCP_DISCOVER)
    packet.vendorInfo.unused[0] = 0xff; /* stopper */


  setMACAddress(&packet.vendorInfo.hwSrc, &Net.MACAddr);
  setMACAddress(&packet.clientHwAddr,     &Net.MACAddr);


  if(type == DHCP_REQUEST){

    // IPアドレス
    packet.vendorInfo.unused[0] = DHCP_OPTION_REQUEST_IP_ADDR;
    packet.vendorInfo.unused[1] = DHCP_OPTION_REQUEST_IP_ADDR_LENGTH;

    *((u32 *) &packet.vendorInfo.unused[2]) = Net.IPAddr;
    he2ne(&packet.vendorInfo.unused[2], sizeof(u32));

    packet.vendorInfo.unused[6] = 0xff; /* stopper */
  }
    

  /*  ネットワーク・バイトオーダに変換  */
  he2ne(&packet.id,                     sizeof(u32));
  he2ne(&packet.sec,                    sizeof(u16));
  he2ne(&packet.IPClient,               sizeof(u32));
  he2ne(&packet.IPUser,                 sizeof(u32));
  he2ne(&packet.IPGateway,              sizeof(u32));
  he2ne(&packet.vendorInfo.magicCode,   sizeof(u32));

  sendUDPPacket(0xffffffff ,68, 67, &packet, 300);
  return;
}




/*
*  Function: receiveDHCPPacket
*
*    DHCPパケットを受信する
*
*  Parameters:
*
*    Packet    -  DHCPのパケット
*
*  Returns:
*
*    なし
*
*/

void receiveDHCPPacket (struct DHCPPacket *Packet){

  char *options;
  bool isOFFERPacket = false;



  ne2he(&Packet->IPUser,    sizeof(u32));
  Net.requestedIPAddr = Packet->IPUser;


  //オプションを読んでいく
  for(options = (void *) &Packet->vendorInfo.magicCode+4;;){

     //
     //  タイプ
     //

     if(options[0] == DHCP_OPTION_TYPE){


       //  OFFERパケット
       if(options[2] == DHCP_OFFER){

         isOFFERPacket = true;
       }


       //  ACKパケット
       if(options[2] == DHCP_ACK){

         isOFFERPacket  = false;
         Net.gotIPAddr = true;
         Net.IPAddr    = Net.requestedIPAddr;

         break;
       }

       options+=2+options[1];
       continue;
     }


     //
     //  ルータのIPアドレス
     //

     if(options[0] == DHCP_OPTION_ROUTER_IP_ADDR){

       Net.RouterIPAddr = *((u32 *) &options[2]);
       ne2he(&Net.RouterIPAddr, sizeof(u32));

       options+=2+options[1];
       continue;
     }


     //
     //  DNSサーバのIPアドレス
     //

     if(options[0] == DHCP_OPTION_DNS_SERVER_IP_ADDR){

       Net.DNSServerIPAddr = *((u32 *) &options[2]);
       ne2he(&Net.DNSServerIPAddr, sizeof(u32));

       options+=2+options[1];
       continue;
     }


     //
     //  サブネットマスク
     //

     if(options[0] == DHCP_OPTION_SUBNETMASK){

       Net.subnetMask = *((u32 *) &options[2]);
       ne2he(&Net.subnetMask, sizeof(u32));

       options+=2+options[1];
       continue;
     }


     //
     //  ストッパー
     //

     if(options[0] == DHCP_OPTION_END)
       break;


     //
     //  パディング
     //

     if(options[0] == DHCP_OPTION_PADDING){
       options++;
       continue;
     }


     //
     //  知らないタイプ
     //
     options+=2+options[1];
  }


  //
  //  REQUESTパケットを送信
  //
  if(isOFFERPacket)
    sendDHCPPacket(DHCP_REQUEST);

  return;
}

