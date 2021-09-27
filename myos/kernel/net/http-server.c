#include "kernel.h"
#include "std.h"
#include "net.h"



/*
*  Function: HTTPServer
*
*    HTTPサーバ
*
*  Parameters:
*
*    ct      -  コネクションのID
*    request -  リクエスト
*
*  Returns:
*
*    なし
*
*/


void HTTPServer (uIndex ct, const char *request){

 char method  [16];
 char path    [32];
 char version [16];
 char *buf = (char *) TEMP_ADDRESS+1000;
 u32 filesize;
 int i=0,j;

#define HTTP_200_HEADER "HTTP/1.1 200 OK\r\nServer: myos\r\nContent-Type: text/html; charset=utf-8\r\nConnection: close\r\n\r\n\0"
#define HTTP_404_HEADER "HTTP/1.1 404 Not Found\r\nServer: myos\r\nConnection: close\r\n\r\n\0"


  //
  //  リクエスト行
  //

  for(j=0; (request[i] != ' ') && (request[i] != '\r') && (j < 15); i++, j++)
    method[j] = request[i];

  i++;
  method[j]  = '\0';

  for(j=0; (request[i] != ' ') && (request[i] != '\r') && (j < 31); i++, j++)
    path[j] = request[i];

  i++;
  path[j]    = '\0';

  for(j=0; (request[i] != ' ') && (request[i] != '\r') && (j < 15); i++, j++)
    version[j] = request[i];

  i++;
  version[j] = '\0';


  // FIXME
  for(i=0; i < 300; i++){
    if(request[i] == '\r' && request[i+1] == '\n' &&
       request[i+2] == '\r' && request[i+3] == '\n')
      break;
  }

  if(!(request[i] == '\r' && request[i+1] == '\n' &&
     request[i+2] == '\r' && request[i+3] == '\n'))
    return;

  //
  //  GET
  //
  if(strcmp("GET", method) == 0){


    for(i=0; i < sizeof(buf); i++)
      buf[i] = 0;

    print("HTTP Server: Access (%s %s %s)\n", method, path, version);
    strcpy((char *) buf, HTTP_200_HEADER);

    // ファイルが指定されていない場合はindex.htmlを送る
    if(strcmp(path, "/") == 0)
      strcpy(path, "/index.html");


    if(!loadFile((void *) buf+sizeof(HTTP_200_HEADER), (void *) path+1, sizeof(buf)-sizeof(HTTP_200_HEADER))){

      print("HTTP Server: file load error \"%s\"\n", (void *) path+1);
      filesize = sizeof(HTTP_404_HEADER);
      strcpy((char *) buf, HTTP_404_HEADER);

    }else{


      filesize  = getFilesize((void *) path+1);
    }

    filesize += 400; // 足りない
    sendDataByTCP(ct, buf, filesize);

    return;
  }



  //
  //  HEAD
  //

  if(strcmp("HEAD", method) == 0){

    filesize = sizeof(HTTP_200_HEADER);

    strcpy((char *) buf, HTTP_200_HEADER);
    sendDataByTCP(ct, buf, filesize);
    return;
  }


  return;
}

