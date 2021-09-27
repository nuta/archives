
#include "homu_main.h"



homu_main_result_t homu_main_setAppRule (
                                          struct homu_main_state* state,
                                          homu_main_userid_t      userid,
                                          char*                   path,
                                          homu_main_resid_t*      res
                                        ){

 homu_main_uintmax_t   i,j;
 homu_main_size_t      resNum;
 homu_main_resid_t*    appres;


  /* count the number of resources */
  for(i=0; res[i] != HOMU_MAIN_RES_NULL; i++);
  resNum = i+1;

  /* look for the app by the path */
  for(i=2; i < HOMU_MAIN_APP_MAX; i++){

    if(state->users[userid].apps[i].path == HOMU_MAIN_NULL)
      continue;

    if(homu_main_strcmp(path, state->users[userid].apps[i].path) == 0)
      break;
  }

  if(i == HOMU_MAIN_APP_MAX){

    /* look for unused "state->users[userud].apps[x]" to add a new app */
    for(
         i=2;
              state->users[userid].apps[i].path != HOMU_MAIN_NULL &&
              i < HOMU_MAIN_APP_MAX                               ;
         i++
       );


    if(i == HOMU_MAIN_APP_MAX){

      /* the number of apps has reached the limit */
      homu_main_throwError(state, userid, HOMU_MAIN_ERR_TOO_MANY_APPS);
      return HOMU_MAIN_FAILURE;

    }else{

      state->users[userid].apps[i].path = homu_main_malloc(state, state->users[userid].apps[i].path, homu_main_strlen(path) + 1);
      homu_main_strcpy(state->users[userid].apps[i].path, path);
      state->users[userid].apps[i].path[homu_main_strlen(path)] = '\0';
    }
  }

  appres = state->users[userid].apps[i].res;


  /* set the new app rule */
  appres = homu_main_malloc(state, appres, sizeof(*appres) * (resNum+1));

  for(j=0; res[j] != HOMU_MAIN_RES_NULL; j++)
    appres[j] = res[j];

  appres[j] = HOMU_MAIN_RES_NULL;

  state->users[userid].apps[i].res = appres;

  return HOMU_MAIN_SUCCESS;
}




homu_main_result_t homu_main_setDefaultAppRule (
                                                 struct homu_main_state* state,
                                                 homu_main_userid_t      userid,
                                                 homu_main_resid_t*      res
                                               ){

 homu_main_uintmax_t i;
 homu_main_size_t    resNum;
 homu_main_resid_t*  defaultres;


  defaultres = state->users[userid].apps[1].res;

  /* count the number of resources */
  for(i=0; res[i] != HOMU_MAIN_RES_NULL; i++);
  resNum = i;

  /* set the new app rule */
  defaultres = homu_main_malloc(state, defaultres, sizeof(*defaultres) * (resNum+1));

  for(i=0; res[i] != HOMU_MAIN_RES_NULL; i++)
    defaultres[i] = res[i];


  defaultres[i] = HOMU_MAIN_RES_NULL;

  state->users[userid].apps[1].res = defaultres;

  return HOMU_MAIN_SUCCESS;
}

