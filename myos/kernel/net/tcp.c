#include "kernel.h"
#include "std.h"
#include "net.h"


void HTTPServer (uIndex ct, const char *request);


/*
*  Function: sendDataByTCP
*
*    TCPによるデータ送信をする
*
*  Parameters:
*
*    ct     -  コネクションのID
*    data   -  データ
*    size   -  データのサイズ
*
*  Returns:
*
*    なし
*
*/

void sendDataByTCP (uIndex ct, void *data, size length){


  if(Net.TCP.Connection[ct].windowSize > length){

    Net.TCP.Connection[ct].status = TCP_CONT_FIN_WAIT1;
    Net.TCP.Connection[ct].sendP  = 0;
    sendTCPPacket(&Net.TCP.Connection[ct], TCP_CTRLFLAG_ACK, length+TCP_HEADER_LENGTH*4, (void *) data, false);
    sendTCPPacket(&Net.TCP.Connection[ct], TCP_CTRLFLAG_ACK|TCP_CTRLFLAG_FIN, TCP_HEADER_LENGTH*4, NULL, true);

  }else{


    Net.TCP.Connection[ct].status = TCP_CONT_SENDING;
    Net.TCP.Connection[ct].data   = kmalloc(length);
    Net.TCP.Connection[ct].sendP  = 0;
    Net.TCP.Connection[ct].length = length;
    memcpy(Net.TCP.Connection[ct].data, data, length);

    Net.TCP.Connection[ct].sendP  = 400;
    sendTCPPacket(&Net.TCP.Connection[ct], TCP_CTRLFLAG_ACK, 400+TCP_HEADER_LENGTH*4, (void *) data, false);
  }

  return;
}




/*
*  Function: receiveTCPPacket
*
*    TCPパケットを受信する
*
*  Parameters:
*
*    Packet -  TCPパケット
*    IPSrc  -  送信元のIPアドレス
*    size   -  パケットのサイズ
*
*  Returns:
*
*    なし
*
*/

void receiveTCPPacket (struct TCPPacket *Packet, IPAddress IPSrc, u16 size){

 uCount ct;


  ne2he(&Packet->info,        sizeof(u16));
  ne2he(&Packet->portSrc,     sizeof(u16));
  ne2he(&Packet->portDest,    sizeof(u16));
  ne2he(&Packet->windowSize,  sizeof(u16));
  ne2he(&Packet->ackNum,      sizeof(u32));
  ne2he(&Packet->seqNum,      sizeof(u32));

  // ポート番号からコネクションを探す
  ct = getTCPContBySrcPort(Packet->portDest);



  //
  // SYNパケットを受信 (スリーウェイ・ハンドシェイク)
  //

  if(Packet->info&TCP_CTRLFLAG_SYN){

    Net.TCP.Connection[ct].IPDest       = IPSrc;
    Net.TCP.Connection[ct].portDest     = Packet->portSrc;
    Net.TCP.Connection[ct].portSrc      = Packet->portDest;

    Net.TCP.Connection[ct].seqNum       = 0x1000;

    Net.TCP.Connection[ct].ackNum       = Packet->seqNum+1;
    Net.TCP.Connection[ct].windowSize   = (Packet->windowSize > Net.TCP.Connection[ct].windowSize)?
                                           Net.TCP.Connection[ct].windowSize:Packet->windowSize;

    sendTCPPacket(&Net.TCP.Connection[ct], TCP_CTRLFLAG_ACK|TCP_CTRLFLAG_SYN, TCP_HEADER_LENGTH*4, NULL, true);

    return;
  }




  //
  // FINパケットを受信 (コネクションの切断)
  //

  if(Packet->info&TCP_CTRLFLAG_FIN){

    Net.TCP.Connection[ct].seqNum       = Packet->ackNum;
    Net.TCP.Connection[ct].ackNum       = Packet->seqNum+1;
    Net.TCP.Connection[ct].windowSize   = (Packet->windowSize > Net.TCP.Connection[ct].windowSize)?
                                           Net.TCP.Connection[ct].windowSize:Packet->windowSize;


    sendTCPPacket(&Net.TCP.Connection[ct], TCP_CTRLFLAG_ACK|TCP_CTRLFLAG_FIN, TCP_HEADER_LENGTH*4, NULL, true);
  }




  //
  // ACKパケットを受信 (データ送信中)
  //

  if(Net.TCP.Connection[ct].status == TCP_CONT_SENDING){

    Net.TCP.Connection[ct].seqNum       = Packet->ackNum;
    Net.TCP.Connection[ct].ackNum       = Packet->seqNum;
    Net.TCP.Connection[ct].windowSize   = (Packet->windowSize > Net.TCP.Connection[ct].windowSize)?
                                           Net.TCP.Connection[ct].windowSize:Packet->windowSize;


    if((Net.TCP.Connection[ct].length-Net.TCP.Connection[ct].sendP) < Net.TCP.Connection[ct].windowSize){
      Net.TCP.Connection[ct].status = TCP_CONT_FIN_WAIT1;


      sendTCPPacket(&Net.TCP.Connection[ct], TCP_CTRLFLAG_ACK|TCP_CTRLFLAG_FIN,
                    (Net.TCP.Connection[ct].length-Net.TCP.Connection[ct].sendP)+TCP_HEADER_LENGTH*4,
                    (void *) Net.TCP.Connection[ct].data+Net.TCP.Connection[ct].sendP, false);
    }else{


      sendTCPPacket(&Net.TCP.Connection[ct], TCP_CTRLFLAG_ACK, Net.TCP.Connection[ct].windowSize+TCP_HEADER_LENGTH*4,
                    (void *) Net.TCP.Connection[ct].data+Net.TCP.Connection[ct].sendP, false);

      Net.TCP.Connection[ct].sendP += Net.TCP.Connection[ct].windowSize;
    }



    return;
  }


    /*  FIXME: HTML送信  */

    Net.TCP.Connection[ct].status       = TCP_CONT_ACK_WAIT;
    Net.TCP.Connection[ct].seqNum       = Packet->ackNum;
    Net.TCP.Connection[ct].ackNum       = Packet->seqNum+1;
    Net.TCP.Connection[ct].windowSize   = (Packet->windowSize > Net.TCP.Connection[ct].windowSize)?
                                           Net.TCP.Connection[ct].windowSize:Packet->windowSize;

    HTTPServer(ct, (const char *) Packet->data);

    return;
}




/*
*  Function: getTCPContBySrcPort
*
*    ポート番号からTCPのコネクションのIDを取得する
*
*  Parameters:
*
*    port -  ポート
*
*  Returns:
*
*    TCPのコネクションのID
*
*/

unsigned int getTCPContBySrcPort (Port port){

 unsigned int  i;


  for(i=1; (Net.TCP.Connection[i].portSrc != port) && (i < TCP_CONT_MAX); i++)

  if(i == TCP_CONT_MAX)
    return 1;

  return i;
}




/*
*  Function: createTCPConnection
*
*    TCPのコネクションを作成する
*
*  Parameters:
*
*    cont     -  コネクション
*    IPDest   -  送信先のIPアドレス
*    portDest -  送信先のポート
*    portSrc  -  送信元のポート
*
*
*  Returns:
*
*    成功した場合はコネクションのID、失敗した場合は0を返します。
*
*/

uIndex createTCPConnection  (IPAddress IPDest, Port portDest, Port portSrc){

 uCount  i;


  for(i=1; (Net.TCP.Connection[i].portSrc == 0) && (i < TCP_CONT_MAX); i++)

  Net.TCP.Connection[i].status      = TCP_CONT_CLOSED;
  Net.TCP.Connection[i].IPDest      = IPDest;
  Net.TCP.Connection[i].portDest    = portDest;
  Net.TCP.Connection[i].portSrc     = portSrc;
  Net.TCP.Connection[i].seqNum      = 0x1234;
  Net.TCP.Connection[i].ackNum      = 0;
  Net.TCP.Connection[i].windowSize  = TCP_DEFAULT_WINDOW_SIZE;


  return (i == TCP_CONT_MAX)? 0:i;
}




/*
*  Function: closeTCPConnection
*
*    TCPのコネクションを切断する
*
*  Parameters:
*
*    i -  コネクションのID
*
*  Returns:
*
*    なし
*
*/

void closeTCPConnection (uIndex i){


  Net.TCP.Connection[i].status  = TCP_CONT_FIN_WAIT1;
  sendTCPPacket(&Net.TCP.Connection[i], TCP_CTRLFLAG_ACK|TCP_CTRLFLAG_FIN, TCP_HEADER_LENGTH*4, NULL, true);

  return;
}




/*
*  Function: sendTCPPacket
*
*    TCPパケットを送信する
*
*  Parameters:
*
*    cont         -  コネクション
*    ctrlBits     -  コントロールビット
*    size         -  データのサイズ
*    data         -  データ
*    isHeaderOnly -  ヘッダのみを送信するか
*
*  Returns:
*
*    なし
*
*/

void sendTCPPacket (struct TCPConnection *cont, u8 ctrlBits, int size, void *data, bool isHeaderOnly){


 int i;
 u16 tmp;
 u16 *p;
 u32 checksum;
 struct TCPPacket Packet;

  p= (u16*) &Packet;


  Packet.portSrc     = cont->portSrc;
  Packet.portDest    = cont->portDest;
  Packet.seqNum      = cont->seqNum;
  Packet.ackNum      = cont->ackNum;
  Packet.windowSize  = cont->windowSize;
  Packet.urgPointer  = 0;
  Packet.info        = ((TCP_HEADER_LENGTH)<<12)+ctrlBits;
  Packet.checksum    = 0;

  if(!isHeaderOnly)
    memcpy(&Packet.data, data, size-(TCP_HEADER_LENGTH*4));


  /*  ネットワーク・バイトオーダに変換  */

  he2ne(&Packet.portSrc,     sizeof(u16));
  he2ne(&Packet.portDest,    sizeof(u16));
  he2ne(&Packet.seqNum,      sizeof(u32));
  he2ne(&Packet.ackNum,      sizeof(u32));
  he2ne(&Packet.windowSize,  sizeof(u16));
  he2ne(&Packet.urgPointer,  sizeof(u16));
  he2ne(&Packet.info,        sizeof(u16));
  he2ne(&Packet.checksum,    sizeof(u16));



  /*  チェックサム計算  */
  checksum=0;

  /* 擬似ヘッダ */
  tmp = Net.IPAddr&0xffff;
  he2ne(&tmp, sizeof(u16));checksum += tmp;

  tmp = (Net.IPAddr>>16)&0xffff;
  he2ne(&tmp, sizeof(u16));checksum += tmp;

  tmp = cont->IPDest&0xffff;
  he2ne(&tmp, sizeof(u16));checksum += tmp;

  tmp = (cont->IPDest>>16)&0xffff;
  he2ne(&tmp, sizeof(u16));checksum += tmp;

  tmp = 0x0006;
  he2ne(&tmp, sizeof(u16));checksum += tmp;

  tmp = size;
  he2ne(&tmp, sizeof(u16));checksum += tmp;

  for(i=0; i < ((TCP_HEADER_LENGTH)*4)/2; i++)
    checksum += p[i];


  if(!isHeaderOnly){

    p = (u16 *) &Packet.data;


    for(i=0; i < ((size-(TCP_HEADER_LENGTH*4))/2); i++){

      tmp = p[i];
      checksum += tmp;
    }
  }

  checksum = (checksum&0xffff)+(checksum>>16);
  Packet.checksum = (~checksum == 0x0000)? 0xffff:~checksum;


  sendIPPacket(cont->IPDest, IP_PROTOCOL_TCP, &Packet, size);

  return;
}

