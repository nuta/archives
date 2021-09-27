
#include "homu_main.h"



homu_main_result_t homu_main_initUser (
                                        struct homu_main_state* state,
                                        homu_main_userid_t      userid
                                      ){

 homu_main_uintmax_t i;


  for(i=0; i < HOMU_MAIN_APP_MAX; i++){

    state->users[userid].apps[i].path = HOMU_MAIN_NULL;
    state->users[userid].apps[i].res  = HOMU_MAIN_NULL;
  }

  for(i=0; i < HOMU_MAIN_USERRES_MAX; i++){

    state->users[userid].userres[i].type = HOMU_MAIN_USERRES_NULL;
    state->users[userid].userres[i].str  = HOMU_MAIN_NULL;
  }

  for(i=0; i < HOMU_MAIN_PROS_MAX; i++){

    state->users[userid].process[i].appid = 0;
  }

  state->users[userid].apps[1].path = "///defaultapprule///";
  state->users[userid].log          = homu_main_malloc(state, HOMU_MAIN_NULL, HOMU_MAIN_LOG_MAX);
  homu_main_strcpy(state->users[userid].log, "");
  state->users[userid].logLen       = 0;
  state->users[userid].mode         = HOMU_MAIN_SLEEP_MODE;

  return HOMU_MAIN_SUCCESS;
}

