
#include "homu_main.h"



#define isdigit(c) (((c=='0')||(c=='1')||(c=='2')||(c=='3')||(c=='4')|| \
                     (c=='5')||(c=='6')||(c=='7')||(c=='8')||(c=='9')) ? 1:0)
#define digit2int(c) (c-48)


unsigned int homu_main_pow (unsigned int base, unsigned int exp){

 unsigned int res, i;


  res = base;

  if(exp == 0)
    return 1;

  for(i=0; i < (exp-1); i++)
    res *= base;

  return res;
}




unsigned int homu_main_str2uint (const char* str){

 unsigned int res = 0;
 signed   int i, j, l;


  for(i=0; isdigit(str[i]) && (str[i] != '\0'); i++);
  i--;

  if(i == 0)
    return digit2int(str[0]);

  for(j=i, l=0; j >= 0; j--, l++)
    res += digit2int(str[j]) * homu_main_pow(10, l);

 return res;
}




homu_main_size_t homu_main_strlen (const char* str){

 homu_main_size_t i;

  for(i=0; str[i] != '\0'; i++);

  return i;
}




int homu_main_strcmp (const char* str1, const char* str2){

 homu_main_uintmax_t i;


  for(i=0; (str1[i] == str2[i]) && (str1[i] != '\0') && (str2[i] != '\0'); i++);

  return (((str1[i] == '\0') && (str2[i] == '\0'))? 0 : 1);
}




char* homu_main_strcpy (char* dest, const char* src){

 homu_main_uintmax_t i;


  for(i=0; src[i] != '\0'; i++){

    dest[i] = src[i];
  }

  dest[i] = '\0';
  return dest;
}




void* homu_main_memcpy (void* dest, const void* src, homu_main_size_t num){

 homu_main_size_t i;

  for(i=0; i < num; i++){

    *((homu_main_uint8_t *) dest+i) = *((homu_main_uint8_t *) src+i);
  }

  return dest;
}

