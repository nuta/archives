
#include "homu_main.h"

static const char* errorStr [] = {

  "ERROR   NULL (this is not an error)",
  "ERROR   TOO_MANY_APPS",
  "ERROR   TOO_MANY_PROCESSES",
  "WARNING QUERY_INVALID_ARGUMENTS_NUM",
  "WARNING QUERY_INVALID_ARGUMENT",
  "WARNING QUERY_UNKNOWN_COMMAND"
};


void homu_main_throwError (
                            struct homu_main_state* state,
                            homu_main_userid_t      userid,
                            enum homu_main_errType  err
                          ){

  homu_main_writeLog(state, userid, errorStr[err]);

  return;
}
