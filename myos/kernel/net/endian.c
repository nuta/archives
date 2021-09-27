#include "kernel.h"
#include "net.h"


/*
*
*  Function: he2ne
*
*    ホストバイトオーダの数値をネットワークバイトオーダに変換する
*
*  Parameters:
*
*    data  - データへのポインタ
*    size  - データのサイズ
*
*  Returns:
*
*    なし
*
*/

void he2ne (void *data, int size){

 unsigned char  tmp;
 unsigned int   i;

  /*  2バイト  */
  if(size == 2){

    tmp                 = *((char *) data);
    *((char *) data)    = *((char *) data+1);
    *((char *) data+1)  = tmp;

  /*  4バイト  */
  }else if(size == 4){

    for(i=0; i < 2; i++){
      tmp                     = *((char *) data+i);
      *((char *) data+i)      = *((char *) data+(3-i));
      *((char *) data+(3-i))  = tmp;
    }
  }

  return;
}




/*
*
*  Function: ne2he
*
*    ネットワークバイトオーダの数値をホストバイトオーダに変換する
*
*  Parameters:
*
*    data  - データへのポインタ
*    size  - データのサイズ
*
*  Returns:
*
*    なし
*
*/

void ne2he (void *data, int size){

  he2ne(data, size);
  return;
}

