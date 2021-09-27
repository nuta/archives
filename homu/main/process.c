#include "homu_main.h"



homu_main_appid_t homu_main_getAppIdByPath (
                                             struct homu_main_state* state,
                                             homu_main_userid_t      userid,
                                             char*                   path
                                           ){

 homu_main_uintmax_t i;


  /* look for the app rule by the path */
  for(i=2; i < HOMU_MAIN_APP_MAX; i++){

    if(state->users[userid].apps[i].path == HOMU_MAIN_NULL)
      continue;

    if(homu_main_strcmp(path, state->users[userid].apps[i].path) == 0)
      break;
  }

  return ((i == HOMU_MAIN_APP_MAX)? 1 : i);
}



/* for fork() */
homu_main_prosid_t homu_main_copyProcess (
                                           struct homu_main_state* state,
                                           homu_main_userid_t      userid,
                                           homu_main_prosid_t      origProsId
                                         ){

  homu_main_uintmax_t i;
  homu_main_appid_t   origAppId;


  origAppId = state->users[userid].process[origProsId].appid;

  /* look for an unused space */
  for(
       i=1;
            state->users[userid].process[i].appid != 0 &&
            i < HOMU_MAIN_PROS_MAX                     ;
       i++
     );

  if(i == HOMU_MAIN_PROS_MAX){

    /* the number of processes has reached the limit */
    homu_main_throwError(state, userid, HOMU_MAIN_ERR_TOO_MANY_PROCESSES);
    return 0;
  }


  state->users[userid].process[i].appid = (origAppId == 0)? 1 : origAppId;

  return i;
}




void homu_main_addCopiedProcess (
                                  struct homu_main_state* state,
                                  homu_main_userid_t      userid,
                                  char*                   path,
                                  homu_main_prosid_t      prosid
                                ){

  state->users[userid].process[prosid].appid = homu_main_getAppIdByPath(state, userid, path);
  return;
}




homu_main_prosid_t homu_main_addProcess (
                                          struct homu_main_state* state,
                                          homu_main_userid_t      userid,
                                          char*                   path
                                        ){

  homu_main_uintmax_t i;


  /* look for an unused space */
  for(
       i=1;
            state->users[userid].process[i].appid != 0 &&
            i < HOMU_MAIN_PROS_MAX                     ;
       i++
     );

  if(i == HOMU_MAIN_PROS_MAX){

    /* the number of processes has reached the limit */
    homu_main_throwError(state, userid, HOMU_MAIN_ERR_TOO_MANY_PROCESSES);
    return 0;
  }

  state->users[userid].process[i].appid = homu_main_getAppIdByPath(state, userid, path);

  return i;
}




void homu_main_delProcess (
                            struct homu_main_state* state,
                            homu_main_userid_t      userid,
                            homu_main_prosid_t      prosid
                          ){


  state->users[userid].process[prosid].appid = 0;
  return;
}

