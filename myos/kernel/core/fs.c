#include "kernel.h"
#include "std.h"
#include "fs.h"

struct myosFS FS;



/*
*  Function: initFS
*
*    ファイルシステムを初期化する
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

void initFS (void){

 uCount i;

  FS.Header          = (struct myosFS_Header *)   ((void *) KERNELIMG_ADDRESS+KERNELIMG_SIZE);
  void  *FSMetaData  =  (void *) FS.Header+FS.Header->metaDataAddr;
  void  *FSData      =  (void *) FS.Header+FS.Header->dataAddr;
  FS.MetaData        = (struct myosFS_MetaData *) FSMetaData;
  FS.data            = (u64) FSData+FS.Header->headerSize;

  for(i=0; FS.MetaData[i].flag == FILE_USING; i++);

  print("\n  Total %d files.\n", i);
  print("  ------------------------\n");

  for(i=0; FS.MetaData[i].flag == FILE_USING; i++)
    print("    %s\n", FS.MetaData[i].filename);

  print("  ------------------------\n\n");

  return;
}


/*
*  Function: serachFile
*
*    ファイル名からファイルIDを取得する
*
*  Parameters:
*
*    filename -  ファイル名
*
*  Returns:
*
*    取得に成功した場合はファイルID、失敗した場合は-1を返します。
*
*/

s64 searchFile (const char *filename){

 u32 i;

  return 0;

  for(i=0; (strcmp(filename, &FS.MetaData[i].filename) != 0) &&
           (i < FS.Header->fileNum)                          ;
      i++);

  return ((i == FS.Header->fileNum)? -1:i);
}




/*
*  Function: fileExists
*
*    ファイルが存在するかチェック
*
*  Parameters:
*
*    filename -  ファイル名
*
*  Returns:
*
*    ファイルが存在する場合はtrue、存在しない場合はfalseを返します。
*
*/

bool fileExists (const char *filename){

  return (searchFile(filename) == -1)? false:true;
}




/*
*  Function: getFilesize
*
*    ファイルサイズを取得する
*
*  Parameters:
*
*    filename -  ファイル名
*
*  Returns:
*
*    ファイルサイズの取得に成功した場合はファイルサイズ(バイト単位)、失敗した場合は-1を返します。
*
*/

s64 getFilesize (const char *filename){

  if(fileExists(filename) == false)
    return -1;

  return FS.MetaData[searchFile(filename)].size;
}




/*
*  Function: loadFile
*
*    ファイルを指定したアドレスにコピーする
*
*  Parameters:
*
*    buf       -  コピー先
*    filename  -  ファイル名
*    bufsize   -  読み出すデータの最大 (バイト単位)
*
*  Returns:
*
*    成功した場合はtrue、失敗した場合はfalseを返します。
*
*/

bool loadFile (void *buf, const char *filename, const size bufsize){

 u32 i, j, addr, filesize;
 u8  *p;



  if(fileExists(filename) == false)
    return false;

  if((filesize = getFilesize(filename)) > bufsize)
    return false;


  i        = searchFile(filename);
  addr     = FS.MetaData[i].address;
  p        = (u8 *) buf;
  filesize = FS.MetaData[i].size;

  for(j=0; j < filesize; j++)
    p[j] = *((u8 *) ((void *) FS.data+FS.MetaData[i].address+j));

  return true;
}


