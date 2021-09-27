
#include "homu_main.h"



homu_main_result_t homu_main_setUserRes (
                                          struct homu_main_state*     state,
                                          homu_main_userid_t          userid,
                                          homu_main_resid_t           resid,
                                          enum homu_main_userresType  type,
                                          char*                       str
                                        ){

 struct homu_main_userres* res;


  res = &state->users[userid].userres[resid];

  res->type = type;
  res->str  = homu_main_malloc(
                                state,
                                res->str,
                                homu_main_strlen(str) + 1
                              );

  homu_main_strcpy(res->str, str);
  res->str[homu_main_strlen(str)] = '\0';

  return HOMU_MAIN_SUCCESS;
}

