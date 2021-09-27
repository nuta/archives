#include "homu_main.h"



homu_main_judge_t homu_main_judge (
                                    struct homu_main_state* state,
                                    homu_main_userid_t      userid,
                                    homu_main_prosid_t      prosid,
                                    homu_main_resid_t       resid,
                                    const char*             str
                                  ){

 homu_main_uintmax_t i;
 homu_main_resid_t*  appRes;


  appRes = state->users[userid].apps[state->users[userid].process[prosid].appid].res;


  /*
   * check whether _resid_ resource is specified as an User Resource
   */

  /* file */
  if((resid == HOMU_MAIN_RES_FS_READ) || (resid == HOMU_MAIN_RES_FS_WRITE)){

    for(
         i=1;
              state->users[userid].userres[i].type != HOMU_MAIN_USERRES_NULL &&
              i < HOMU_MAIN_USERRES_MAX                                      ;
         i++
       ){

      if(
          state->users[userid].userres[i].type == HOMU_MAIN_USERRES_FILE  &&
	  homu_main_strcmp(state->users[userid].userres[i].str, str) == 0
        ){

        if(homu_main_judge(state, userid, prosid, HOMU_MAIN_RES_USERRES_BEGINNING + i, "") == HOMU_MAIN_DENY)
          return HOMU_MAIN_DENY;
      }
    }
  }

  
  /* allow if _prosid_ app's resid list is null (not specified by user) */
  if(appRes == HOMU_MAIN_NULL)
    return HOMU_MAIN_ALLOW;


  /* look for resid in the app's resid list */
  for(
       i=0;
            appRes[i] != resid              &&
            appRes[i] != HOMU_MAIN_RES_NULL ;
       i++
     );


  return ((appRes[i] == HOMU_MAIN_RES_NULL)? HOMU_MAIN_DENY : HOMU_MAIN_ALLOW);
}
