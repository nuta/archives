
#include "homu_main.h"



const char* homu_main_readLog (
                                struct homu_main_state* state,
                                homu_main_userid_t      userid
                              ){

  return state->users[userid].log;
}




void homu_main_writeLog (
                          struct homu_main_state* state,
                          homu_main_userid_t      userid,
                          const char*             msg
                        ){


  if((homu_main_strlen(state->users[userid].log) + homu_main_strlen(msg)) > HOMU_MAIN_LOG_MAX){

    state->users[userid].logLen = 0;
  }

  homu_main_strcpy(&state->users[userid].log[state->users[userid].logLen], msg);
  state->users[userid].log[state->users[userid].logLen + homu_main_strlen(msg)]   = '\n';
  state->users[userid].log[state->users[userid].logLen + homu_main_strlen(msg)+1] = '\0';
  state->users[userid].logLen += homu_main_strlen(msg) + 1;

  return;
}
