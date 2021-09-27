#include "kernel.h"


/*
*  Function: moveCursor
*
*    カーソルを移動する
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

void moveCursor (void){

  unsigned int cursor = (Screen.y*Screen.xMax)+Screen.x;

  asm_out8(0x03d4, 15);
  asm_out8(0x03d5, (s8) cursor&0xff);
  asm_out8(0x03d4, 14);
  asm_out8(0x03d5, (s8) (cursor>>8)&0xff);

 return;
}




/*
*  Function: printchar
*
*    文字を表示する
*
*  Parameters:
*
*    ch    -  文字
*    color -  色
*
*  Returns:
*
*    なし
*
*/

void printchar (const char ch, const unsigned char color){

 unsigned int x,y;
 char *vram = (char *) Screen.VRAMAddress;


  /*  改行  */
  if(ch == '\n'){
    Screen.y++;
    Screen.x=0;

    /*  画面いっぱいになった  */
    if(Screen.y >= Screen.yMax){

      /*  一行ずらす  */
      for(y=0; y < Screen.yMax; y++){
        for(x=0; x < Screen.xMax; x++){
          vram[(y*(Screen.xMax*2))+(x*2)]   = vram[((y+1)*(Screen.xMax*2))+(x*2)];
          vram[(y*(Screen.xMax*2))+(x*2)+1] = vram[((y+1)*(Screen.xMax*2))+(x*2)+1];
        }
      }


      Screen.y = Screen.yMax-1;
    }

    moveCursor();
    return;
  }



  /*  普通の文字  */
  vram[(Screen.y*(Screen.xMax*2))+(Screen.x*2)]   = ch;
  vram[(Screen.y*(Screen.xMax*2))+(Screen.x*2)+1] = (char) color;


  Screen.x++;


  /*  行いっぱいになった  */
  if(Screen.x > Screen.xMax){
    Screen.y++;
    Screen.x=0;


    /*  画面いっぱいになった  */
    if(Screen.y >= Screen.yMax){

      /*  一行ずらす  */
      for(y=0; y < Screen.yMax; y++){
        for(x=0; x < Screen.xMax; x++){
          vram[(y*(Screen.xMax*2))+(x*2)]   = vram[((y+1)*(Screen.xMax*2))+(x*2)];
          vram[(y*(Screen.xMax*2))+(x*2)+1] = vram[((y+1)*(Screen.xMax*2))+(x*2)+1];
        }
      }


      Screen.y = Screen.yMax-1;
    }
  }


  moveCursor();
  return;
}




/*
*  Function: print
*
*    書式文字列に従い文字列を表示する
*
*  Parameters:
*
*    format   -  書式文字列
*
*  Returns:
*
*    なし
*
*/

void print (const char *format, ...){

 va_list vargs;

  va_start(vargs, format); 
  vprintf(format, vargs);
  va_end(vargs); 

  return;
}


