
#include "homu_main.h"



enum homu_main_mode homu_main_getMode (
                                        struct homu_main_state* state,
                                        homu_main_userid_t      userid
                                      ){


  return state->users[userid].mode;
}




homu_main_result_t homu_main_setMode (
                                       struct homu_main_state* state,
                                       homu_main_userid_t      userid,
                                       enum homu_main_mode     mode
                                     ){


  if(mode == HOMU_MAIN_SLEEP_MODE){

    homu_main_writeLog(state, userid, "MODE_CHANGED SLEEP");

  }else if(mode == HOMU_MAIN_OBSERVE_MODE){

    homu_main_writeLog(state, userid, "MODE_CHANGED OBSERVE");

  }else if(mode == HOMU_MAIN_INVOKE_MODE){

    homu_main_writeLog(state, userid, "MODE_CHANGED INVOKE");
  }

  state->users[userid].mode = mode;
  return HOMU_MAIN_SUCCESS;
}

