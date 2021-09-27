
#include "homu_main.h"



homu_main_result_t homu_main_query (
                                     struct homu_main_state* state,
                                     homu_main_userid_t      userid,
                                     const char*             query
                                   ){

 homu_main_uintmax_t        i, j, k;
 homu_main_size_t           argNum;
 enum homu_main_userresType userrestype;
 homu_main_resid_t*   res;
 homu_main_bool_t     breaknow;
 homu_main_bool_t     isString;
 char*  str;
 char*  cmd;
 char** args;


  res = homu_main_malloc(state, HOMU_MAIN_NULL, sizeof(*res) * HOMU_MAIN_USERRES_MAX);

  /* copy the query string */
  str = homu_main_malloc(state, HOMU_MAIN_NULL, homu_main_strlen(query) + 1);
  homu_main_strcpy(str, query);
  str[homu_main_strlen(query)] = '\0';

  /* count the number of arguments */
  argNum   = 0;
  isString = HOMU_MAIN_FALSE;

  for(i=0; str[i] != '\0'; i++){

    if(str[i] == '"')
      isString = (isString)? HOMU_MAIN_FALSE : HOMU_MAIN_TRUE;

    if(!isString && str[i] == ' ')
      argNum++;
  }

  args = homu_main_malloc(state, HOMU_MAIN_NULL, (argNum+1) * sizeof(*args));



  /*
  *
  *  parse
  *
  */

  /* command */
  cmd = &str[0];
  for(i=0; (str[i] != ' ') && (str[i] != '\0'); i++);

  /* arguments */
  for(j=0; str[i] != '\0'; j++){

    str[i]  = '\0';
    args[j] = &str[i+1];

    if(str[i+1] == '\0')
      break;

    for(i++; (str[i] != ' ') && (str[i] != '\0'); i++);
  }

  args[j] = HOMU_MAIN_NULL;


  /*
  *
  *  execute
  *
  */



  /*
  *  set_mode
  */

  if(homu_main_strcmp(cmd, "set_mode") == 0){

    if(argNum != 1){

      homu_main_throwError(state, userid, HOMU_MAIN_ERR_QUERY_INVALID_ARGUMENTS_NUM);
      goto ret_failure;
    }

    /* sleep mode */
    if(homu_main_strcmp(args[0], "sleep") == 0){
      homu_main_setMode(state, userid, HOMU_MAIN_SLEEP_MODE);

    /* observe mode */
    }else if(homu_main_strcmp(args[0], "observe") == 0){
      homu_main_setMode(state, userid, HOMU_MAIN_OBSERVE_MODE);

    /* invoke mode */
    }else if(homu_main_strcmp(args[0], "invoke") == 0){
      homu_main_setMode(state, userid, HOMU_MAIN_INVOKE_MODE);

    /* unknown mode */
    }else{

      homu_main_throwError(state, userid, HOMU_MAIN_ERR_QUERY_INVALID_ARGUMENT);
      goto ret_failure;
    }


  /*
  *  set_apprule
  */

  }else if(homu_main_strcmp(cmd, "set_apprule") == 0){

    if(argNum != 2){

      homu_main_throwError(state, userid, HOMU_MAIN_ERR_QUERY_INVALID_ARGUMENTS_NUM);
      goto ret_failure;
    }

    for(i=0, k=0, breaknow=HOMU_MAIN_FALSE; breaknow == HOMU_MAIN_FALSE; k++){

      for(j=i; (args[1][i] != ',') && (args[1][i] != '\0'); i++);

      if(args[1][i] == '\0')
        breaknow = HOMU_MAIN_TRUE;

      args[1][i] = '\0';
      i++;

      res[k] = homu_main_str2uint(&args[1][j]);
   }

    res[k] = HOMU_MAIN_RES_NULL;

    homu_main_setAppRule(state, userid, args[0], res);


  /*
  *  set_defaultapprule
  */

  }else if(homu_main_strcmp(cmd, "set_defaultapprule") == 0){

    if(argNum != 1){

      homu_main_throwError(state, userid, HOMU_MAIN_ERR_QUERY_INVALID_ARGUMENTS_NUM);
      goto ret_failure;
    }

    for(i=0, k=0, breaknow=HOMU_MAIN_FALSE; breaknow == HOMU_MAIN_FALSE; k++){

      for(j=i; (args[0][i] != ',') && (args[0][i] != '\0'); i++);

      if(args[0][i] == '\0')
        breaknow = HOMU_MAIN_TRUE;

      args[0][i] = '\0';
      i++;

      res[k] = homu_main_str2uint(&args[0][j]);
   }

    res[k] = HOMU_MAIN_RES_NULL;

    homu_main_setDefaultAppRule(state, userid, res);


  /*
  *  reset_rule
  */

  }else if(homu_main_strcmp(cmd, "reset_rule") == 0){

    homu_main_reset(state, userid);


  /*
  *  set_userres
  */

  }else if(homu_main_strcmp(cmd, "set_userres") == 0){

    if(argNum != 3){

      homu_main_throwError(state, userid, HOMU_MAIN_ERR_QUERY_INVALID_ARGUMENTS_NUM);
      goto ret_failure;
    }

    if(homu_main_strcmp(args[1], "FILE") == 0){

      userrestype = HOMU_MAIN_USERRES_FILE;

    }else{

      homu_main_throwError(state, userid, HOMU_MAIN_ERR_QUERY_INVALID_ARGUMENT);
      goto ret_failure;
    }

    homu_main_setUserRes(state, userid, homu_main_str2uint(args[0]), userrestype, args[2]);


  /*
  *  unknown command
  */
  }else{

    homu_main_throwError(state, userid, HOMU_MAIN_ERR_QUERY_UNKNOWN_COMMAND);
    goto ret_failure;
  }



  homu_main_mrelease(state, str);
  homu_main_mrelease(state, args);
  return HOMU_MAIN_SUCCESS;

  ret_failure:
  homu_main_mrelease(state, str);
  homu_main_mrelease(state, args);
  return HOMU_MAIN_FAILURE;
}

