#include "kernel.h"



/*
*  Function: vprintf
*
*    書式文字列に従い文字列を表示する
*
*  Parameters:
*
*    format  - 書式文字列
*    args    - 可変長引数
*
*  Returns:
*
*    常に0
*
*/

int vprintf (const char *format, va_list vargs){

 char buf[64];

 char *str;
 int   i,j,k,l;

 char c;
 unsigned int  int10, int16;

 char int10Table[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
 char int16Table[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

 unsigned char textColor = CUI_FONT_COLOR&0x0f;
 unsigned char bgColor   = (CUI_FONT_COLOR>>8)&0x0f;


  for(i=0; format[i] != '\0'; i++){

    /* バッファ0クリア */
    for(k=0; k < 64; k++)
      buf[k] = '\0';

    /*
    *  色指定
    */


    if(format[i] == '\x1b'){

      i++;

      if(format[i] == '['){
        i++;

        if(      strncmp("40", &format[i], 2) == 0){  bgColor = 0x00;     // 前景色: 黒色
        }else if(strncmp("41", &format[i], 2) == 0){  bgColor = 0x04;     // 前景色: 赤色
        }else if(strncmp("42", &format[i], 2) == 0){  bgColor = 0x02;     // 前景色: 緑色
        }else if(strncmp("43", &format[i], 2) == 0){  bgColor = 0x06;     // 前景色: 黄色
        }else if(strncmp("44", &format[i], 2) == 0){  bgColor = 0x01;     // 前景色: 青色
        }else if(strncmp("45", &format[i], 2) == 0){  bgColor = 0x05;     // 前景色: マゼンダ
        }else if(strncmp("46", &format[i], 2) == 0){  bgColor = 0x03;     // 前景色: シアン
        }else if(strncmp("47", &format[i], 2) == 0){  bgColor = 0x07;     // 前景色: 灰色
        }else if(strncmp("49", &format[i], 2) == 0){  bgColor = (CUI_FONT_COLOR>>8)&0x0f; // 前景色をデフォルトに戻す

        }else if(strncmp("30", &format[i], 2) == 0){  textColor = 0x00;   // 背景色: 黒色
        }else if(strncmp("31", &format[i], 2) == 0){  textColor = 0x40;   // 背景色: 赤色
        }else if(strncmp("32", &format[i], 2) == 0){  textColor = 0x20;   // 背景色: 緑色
        }else if(strncmp("33", &format[i], 2) == 0){  textColor = 0x60;   // 背景色: 黄色
        }else if(strncmp("34", &format[i], 2) == 0){  textColor = 0x10;   // 背景色: 青色
        }else if(strncmp("35", &format[i], 2) == 0){  textColor = 0x50;   // 背景色: マゼンダ
        }else if(strncmp("36", &format[i], 2) == 0){  textColor = 0x30;   // 背景色: シアン
        }else if(strncmp("37", &format[i], 2) == 0){  textColor = 0xf0;   // 背景色: 白
        }else if(strncmp("39", &format[i], 2) == 0){  textColor = CUI_FONT_COLOR&0x0f;   // 背景色をデフォルトに戻す

        }else{
          textColor = CUI_FONT_COLOR&0x0f;
          bgColor   = (CUI_FONT_COLOR>>8)&0x0f;
        }
      }

      i+=2;
      continue;
    }




    if(format[i] == '%'){

      i++;

      /*
      *   %自身
      */
      if(format[i] == '%'){

        printchar('%', textColor+bgColor);


      /*
      *   文字
      */
      }else if(format[i] == 'c'){

        printchar((char) va_arg(vargs, int), textColor+bgColor);


      /*
      *   文字列
      */
      }else if(format[i] == 's'){

        str = va_arg(vargs, char*);

        for(k=0; str[k] != '\0'; k++)
          printchar(str[k], textColor+bgColor);


      /*
      *   int型の整数から10進数文字列
      */
      }else if(format[i] == 'd'){

        int10 = (int) va_arg(vargs, int);
        j=0;

        do{

          buf[j]  = int10Table[int10%10];
          int10  /= 10;
          j++;

        }while((int10) > 0);

        buf[j] = '\0';



        /* 数字の順番が逆(リトルエンディアンが関係?)なので正しい順番に直す */
        for(l=1; l < j/2+1; l++){
          c        = buf[j-l];
          buf[j-l] = buf[l-1];
          buf[l-1] = c;
        }


        for(j=0; buf[j] != '\0'; j++)
          printchar(buf[j], textColor+bgColor);

        continue;


      /*
      *   int型の整数から16進数文字列
      */
      }else if(format[i] == 'x'){

        int16 = (int) va_arg(vargs, int);
        j=0;

        do{

          buf[j]  = int16Table[int16%16];
          int16  /= 16;
          j++;

        }while((int16) > 0);

        buf[j] = '\0';


        /* 数字の順番が逆(リトルエンディアンが関係?)なので正しい順番に直す */
        for(l=1; l < j/2+1; l++){
          c        = buf[j-l];
          buf[j-l] = buf[l-1];
          buf[l-1] = c;
        }


        for(j=0; buf[j] != '\0'; j++)
          printchar(buf[j], textColor+bgColor);

        continue;

      }


      for(k=0; buf[k] != '\0'; k++)
        printchar(buf[k], textColor+bgColor);

      continue;

    }else{

      printchar(format[i], textColor+bgColor);

    }
  }

  return 0;
}


