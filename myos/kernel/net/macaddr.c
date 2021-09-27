#include "kernel.h"
#include "net.h"



/*
*  Function: setMacAddress
*
*    MACアドレスをセットする
*
*  Parameters:
*
*    addr1 -  セット先
*    addr2 -  セット元
*
*  Returns:
*
*    なし
*
*/

void setMACAddress (void *addr1, void *addr2){

 u8 *to   = (u8 *) addr1;
 u8 *from = (u8 *) addr2;

  to[0] = from[0];
  to[1] = from[1];
  to[2] = from[2];
  to[3] = from[3];
  to[4] = from[4];
  to[5] = from[5];

  return;
}




/*
*  Function: getMacAddress
*
*    IPアドレスからMACアドレスを取得する
*
*  Parameters:
*
*    IPAddr -  対象となるIPアドレス
*
*  Returns:
*
*   取得したMACアドレス
*
*/


MACAddress *getMACAddress (IPAddress IPAddr){

 unsigned int  i;


  /*  ARPテーブルから検索  */
  for(i=0; i < ARP_CACHE_MAX; i++){

    if(Net.ARP.Cache[i].IPAddr == IPAddr)
      return &Net.ARP.Cache[i].MACAddr;

  }


  /*  ARPリクエストを送信  */
  for(i=1; (Net.ARP.Cache[i].IPAddr != 0) && (i < ARP_CACHE_MAX); i++);


  /* ARPテーブルを初期化  */
  if(ARP_CACHE_MAX == i){

    for(i=1; i < ARP_CACHE_MAX; i++)
      Net.ARP.Cache[i].IPAddr = 0;

    return getMACAddress(IPAddr);
  }

  Net.ARP.Cache[i].IPAddr = IPAddr;
  Net.ARP.Cache[i].status = ARPCACHE_WAIT_REPLY;

  sendARPRequest(IPAddr);


  while(Net.ARP.Cache[i].status == ARPCACHE_WAIT_REPLY);

  return 0;
}

