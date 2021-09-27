/*
*  test program for homu main
*/

#include <stdio.h>
#include <stdlib.h>

#include "homu_main.h"



void* homu_malloc (homu_main_size_t size){

  void *p = malloc((size_t) size);

  if(p == NULL){

    printf("homu_malloc: malloc() returned NULL.");
    exit(EXIT_FAILURE);
  }

  return p;
}



void homu_mfree (void* ptr){

  free(ptr);
  return;
}



void test (
            struct homu_main_state* state,
            homu_main_userid_t      userid,
            const char*             query
         ){

  if(homu_main_query(state, userid, query) == HOMU_MAIN_FAILURE){

    printf("failed to execute query: %s\n", query);
    abort();
  }

  return;
}




int main (void){

 struct homu_main_state* homu;
 homu_main_userid_t      userid  = 1;
 homu_main_prosid_t      prosid1, prosid2;


  homu = (struct homu_main_state *) malloc(sizeof(*homu));

  if(homu_main_init(homu, homu_malloc, homu_mfree) == HOMU_MAIN_FAILURE){

    puts("homu_main_init: failed");
    return 1;
  }

  if(homu_main_initUser(homu, userid) == HOMU_MAIN_FAILURE){

    puts("homu_main_initUser: failed");
    return 1;
  }

  test(homu, userid, "reset_rule");
  test(homu, userid, "set_defaultapprule 1,2,3,4,5,6");

  test(homu, userid, "set_apprule /usr/bin/vim 4,11");

  test(homu, userid, "set_apprule /bin/zsh 11,2,3");
  test(homu, userid, "set_apprule /bin/zsh 4");

  test(homu, userid, "set_userres 1 FILE /homu/db/test");
  test(homu, userid, "set_mode invoke");

  prosid1 = homu_main_addProcess(homu, userid, "/usr/bin/vim");
  prosid2 = homu_main_addProcess(homu, userid, "/bin/zsh");

  if(homu_main_judge(homu, userid, prosid1, HOMU_MAIN_RES_FS_READ,  "/hello")         == HOMU_MAIN_ALLOW)
    puts("1. invalid judge: (ALLOWED) prosid1 (vim) --> FS_READ");
  if(homu_main_judge(homu, userid, prosid2, HOMU_MAIN_RES_FS_WRITE, "/hello")         == HOMU_MAIN_DENY)
    puts("2. invalid judge: (DENIED)  prosid2 (zsh) --> FS_WRITE");
  if(homu_main_judge(homu, userid, prosid2, HOMU_MAIN_RES_FS_READ,  "/homu/db/test")  == HOMU_MAIN_ALLOW)
    puts("3. invalid judge: (ALLOWED) prosid2 (zsh) --> USER_RES(1)");
  if(homu_main_judge(homu, userid, prosid1, HOMU_MAIN_RES_FS_WRITE, "/homu/db/test")  == HOMU_MAIN_DENY)
    puts("4. invalid judge: (DENIED)  prosid1 (vim) --> USER_RES(1)");

  return 0;
}

