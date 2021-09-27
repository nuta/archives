
#include "homu_main.h"



struct homu_main_info* homu_main_getInfo (
                                           struct homu_main_state* state,
                                           homu_main_userid_t      userid,
                                           struct homu_main_info*  info
                                         ){


  info->version         = HOMU_MAIN_VERSION;
  info->mode            = state->users[userid].mode;
  info->appMax          = HOMU_MAIN_APP_MAX;
  info->userresMax      = HOMU_MAIN_USERRES_MAX;
  info->processMax      = HOMU_MAIN_PROS_MAX;

  return info;
}

