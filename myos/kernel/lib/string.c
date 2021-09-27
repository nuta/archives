/*
*
*  Function: memcpy
*
*    データをコピーする
*
*  Parameters:
*
*    to    - コピー先
*    from  - コピー元
*    size  - コピーする量 (バイト単位)
*
*  Returns:
*
*    なし
*
*/

void memcpy (void *to, void *from, unsigned int size){

 unsigned int i;

  for(i=0; i < size; i++)
    *((char *) to+i) = *((char *) from+i);

  return;
}




/*
*  Function: strlen
*
*    文字列の長さを調べる
*
*  Parameters:
*
*    str  - 文字列
*
*  Returns:
*
*    文字数
*
*/

unsigned int strlen (const char *str){

 unsigned int i;

  for(i = 0; str[i] != 0; i++);

  return i;
}




/*
*  Function: strncmp
*
*    文字列を指定した文字数だけ比較する
*
*  Parameters:
*
*    str1  - 比較する文字列
*    str1  - 比較する文字列
*    len   - 比較する文字数
*
*  Returns:
*
*    文字列の先頭から指定医sた文字数までの文字列が等しい(内容が同じ)場合には0、等しくない場合は1を返します。
*
*/

int strncmp (const char *str1 , const char *str2 , unsigned int len){

 unsigned int i;

  for(i=0; (str1[i] == str2[i]) && (i < len); i++);

  if(i == len)
    return 0;
  else
    return 1;
}




/*
*  Function: strcmp
*
*    文字列を比較する
*
*  Parameters:
*
*    str1  - 比較する文字列
*    str2  - 比較する文字列
*
*  Returns:
*
*    文字列が等しい(内容が同じ)場合には0、等しくない場合は1を返します。
*
*/

int strcmp (const char *str1 , const char *str2){

 unsigned int i;

  for(i=0; (str1[i] == str2[i]) && ((str1[i] != '\0') && ((str2[i] != '\0'))); i++);

  if(str1[i] == str2[i])
    return 0;
  else
    return 1;
}



/*
*  Function: strcpy
*
*    文字列をコピーする
*
*  Parameters:
*
*    str1  - コピー先
*    str2  - コピー元
*
*  Returns:
*
*    str1へのポインタ
*
*/

char *strcpy (char *str1, const char *str2){

 unsigned int i;

  for(i=0; str2[i] != '\0'; i++)
    str1[i] = str2[i];

  str1[i] = '\0';
  return str1;
}




/*
*
*  Function: le2be
*
*    リトルエンディアンの数値をビッグエンディアンに変換する
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

void le2be (void *data, int size){

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
*  Function: be2le
*
*    ビッグエンディアンの数値をリトルエンディアンに変換する
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

void be2le (void *data, int size){

  le2be(data, size);
  return;
}

