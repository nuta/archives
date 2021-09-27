
#include "homu_main.h"



void homu_main_reset (struct homu_main_state* state, homu_main_userid_t userid){


 homu_main_uintmax_t i;
 struct homu_main_user* user = &state->users[userid];



  for(i=0; i < HOMU_MAIN_APP_MAX; i++){

    if(user->apps[i].res  != HOMU_MAIN_NULL)
      homu_main_mrelease(state, user->apps[i].res);

    /* appid[1]'s path is "///defaultapprule///" (not allocated by homu_main_malloc) */
    if(i == 1)
      continue;

    if(user->apps[i].path != HOMU_MAIN_NULL)
      homu_main_mrelease(state, user->apps[i].path);
  }



  for(i=0; i < HOMU_MAIN_USERRES_MAX; i++){

    if(user->userres[i].type == HOMU_MAIN_USERRES_NULL)
      continue;

    user->userres[i].type = HOMU_MAIN_USERRES_NULL;

    if(user->userres[i].str != HOMU_MAIN_NULL)
      homu_main_mrelease(state, user->userres[i].str);
  }



  for(i=0; i < HOMU_MAIN_PROS_MAX; i++)
    user->process[i].appid = 0;

  if(user->log != HOMU_MAIN_NULL)
    homu_main_mrelease(state, user->log);

  user->logLen = 0;
  user->mode   = HOMU_MAIN_SLEEP_MODE;

  return;
}

