#include "kernel.h"
#include "std.h"
#include "net.h"



/*
*  Function: resolveDomainByDNS
*
*    DNSを使用し、ドメイン名からIPアドレスを取得する
*
*  Parameters:
*
*    qDomain -  対象となるドメイン
*
*  Returns:
*
*    IPアドレス
*
*/

IPAddress resolveDomainByDNS (const char *qDomain){

 struct DNSPacket  Packet;
 unsigned int      i,j, domainLength, labelNum=0;
 char              domain[250];

  strcpy(domain+1, qDomain);
  domainLength = strlen(domain);

  Packet.Header.id   = 0x1234;
  Packet.Header.flag = DNSFLAG_QR_QUERY|DNSFLAG_OPCODE_STQUERY|DNSFLAG_RD_REC;
  Packet.Header.qdCount = 1;



  // ドメイン名をドットで区切り、先頭に文字の長さを追加
  for(i=0, labelNum=0; domain[i] != '\0'; i++){

    if(domain[i] == '.'){
      domain[i] = '\0';
      labelNum++;
    }
  }

  for(i=0, j=0; i <= labelNum; i++){

    domain[j] = strlen(domain+j+1);
    for(;domain[j] != '\0'; j++);
  }

  memcpy(&Packet.question, domain, domainLength+1);


  // 残りのセクションの項目をセット
  *((unsigned short int *) ((void *) &Packet.question+domainLength+1)) = 1;
  *((unsigned short int *) ((void *) &Packet.question+domainLength+3)) = 1;


  // ネットワーク・バイトオーダに変換
  he2ne(&Packet.Header.id,      sizeof(u16));
  he2ne(&Packet.Header.flag,    sizeof(u16));
  he2ne(&Packet.Header.qdCount, sizeof(u16));
  he2ne(&Packet.Header.anCount, sizeof(u16));
  he2ne(&Packet.Header.nsCount, sizeof(u16));
  he2ne(&Packet.Header.arCount, sizeof(u16));

  he2ne((void *) &Packet.question+domainLength+1, sizeof(u16));
  he2ne((void *) &Packet.question+domainLength+3, sizeof(u16));


  sendUDPPacket(Net.DNSServerIPAddr, DNS_PORT_DEST, DNS_PORT_SRC, &Packet, sizeof(struct DNSPacket));

  return 0x000000; /* FIXME */
}

