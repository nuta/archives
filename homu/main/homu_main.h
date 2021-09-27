
#ifndef homu_main_included_main_h
#define homu_main_included_main_h

#define HOMU_MAIN_VERSION  100

#ifdef HOMU_MAIN_DEBUG
  #include <stdlib.h>
  #include <stdio.h>
  #include <string.h>
#endif



#define HOMU_MAIN_DENY      ('S'|'O'|'N'|'O'|                     \
                             'H'|'I'|'T'|'U'|'Y'|'O'|'U'|'H'|'A'| \
                             'N'|'A'|'I'|'W'|'A')
#define HOMU_MAIN_ALLOW      (~HOMU_MAIN_DENY)
#define HOMU_MAIN_NULL       ((void *) 0)
#define HOMU_MAIN_SUCCESS    (1)
#define HOMU_MAIN_FAILURE    (0)
#define HOMU_MAIN_TRUE       (1)
#define HOMU_MAIN_FALSE      (0)

typedef unsigned long int    homu_main_uintmax_t;
typedef signed long int      homu_main_sintmax_t;
typedef unsigned char        homu_main_uint8_t;
typedef unsigned char        homu_main_bool_t;
typedef homu_main_uintmax_t  homu_main_size_t;
typedef signed int           homu_main_result_t;
typedef homu_main_uintmax_t  homu_main_userid_t;
typedef homu_main_uintmax_t  homu_main_appid_t;
typedef homu_main_uintmax_t  homu_main_prosid_t;
typedef homu_main_uintmax_t  homu_main_resid_t;
typedef homu_main_uintmax_t  homu_main_version_t;
typedef homu_main_sintmax_t  homu_main_judge_t;


#define HOMU_MAIN_USER_MAX         (30)
#define HOMU_MAIN_APP_MAX          (500)
#define HOMU_MAIN_USERRES_MAX      (300)
#define HOMU_MAIN_PROS_MAX         (5000)
#define HOMU_MAIN_LOG_MAX          (2000)
enum homu_main_mode{

  HOMU_MAIN_SLEEP_MODE,
  HOMU_MAIN_OBSERVE_MODE,
  HOMU_MAIN_INVOKE_MODE
};

enum homu_main_errType{

  HOMU_MAIN_ERR_NULL                        = 0,
  HOMU_MAIN_ERR_TOO_MANY_APPS               = 1,
  HOMU_MAIN_ERR_TOO_MANY_PROCESSES          = 2,
  HOMU_MAIN_ERR_QUERY_INVALID_ARGUMENTS_NUM = 3,
  HOMU_MAIN_ERR_QUERY_INVALID_ARGUMENT      = 4,
  HOMU_MAIN_ERR_QUERY_UNKNOWN_COMMAND       = 5
};

enum homu_main_userresType{

  HOMU_MAIN_USERRES_NULL,
  HOMU_MAIN_USERRES_FILE
};

enum homu_main_resType{

  HOMU_MAIN_RES_NULL              = 0,
  HOMU_MAIN_RES_NET_CONNECT       = 1,
  HOMU_MAIN_RES_NET_SERVER        = 2,
  HOMU_MAIN_RES_FS_READ           = 3,
  HOMU_MAIN_RES_FS_WRITE          = 4,
  HOMU_MAIN_RES_SYS_CHANGE        = 5,
  HOMU_MAIN_RES_HW_FULLCTRL       = 6,
  HOMU_MAIN_RES_USERRES_BEGINNING = 10
};


struct homu_main_app{

  char*               path;
  homu_main_resid_t  *res;
};

struct homu_main_userres{

  enum homu_main_userresType  type;
  char*                       str;
};

struct homu_main_process{

  homu_main_appid_t appid;
};

struct homu_main_info{

  enum homu_main_mode     mode;
  homu_main_version_t     version;
  homu_main_size_t        appMax;
  homu_main_size_t        userresMax;
  homu_main_size_t        processMax;
};

struct homu_main_user{

  enum homu_main_mode      mode;

  homu_main_size_t         logLen;
  char*                    log;

  struct homu_main_app      apps    [HOMU_MAIN_APP_MAX];
  struct homu_main_userres  userres [HOMU_MAIN_USERRES_MAX];
  struct homu_main_process  process [HOMU_MAIN_PROS_MAX];
};

struct homu_main_state{

  struct homu_main_user users [HOMU_MAIN_USER_MAX];

  void* (*malloc) (homu_main_size_t size);
  void  (*mfree)  (void* ptr);
};


/* macros */
#ifdef HOMU_MAIN_DEBUG

  #define HOMU_MAIN_DEBUGMSG(msg, userid, appid, prosid, resid) do{ \
                            /* call a function to print a message (e.g. printk). */ \
                            }while(0)

#else

  #define HOMU_MAIN_DEBUGMSG(msg, userid, appid, prosid, resid) do{}while(0)

#endif


/* init.c */
homu_main_result_t homu_main_init (
                                    struct homu_main_state* state,
                                    void* (*malloc) (homu_main_size_t size),
                                    void  (*mfree)  (void* ptr)
                                  );

/* reset.c */

void homu_main_reset (struct homu_main_state* state, homu_main_userid_t userid);

/* query.c */
homu_main_result_t homu_main_query (
                                     struct homu_main_state* state,
                                     homu_main_userid_t      userid,
                                     const char*             query
                                   );

/* user.c */
homu_main_result_t homu_main_initUser (
                                        struct homu_main_state* state,
                                        homu_main_userid_t      userid
                                      );

/* app.c */
homu_main_result_t homu_main_setAppRule (
                                          struct homu_main_state* state,
                                          homu_main_userid_t      userid,
                                          char*                   path,
                                          homu_main_resid_t*      res
                                        );
homu_main_result_t homu_main_setDefaultAppRule (
                                                 struct homu_main_state* state,
                                                 homu_main_userid_t      userid,
                                                 homu_main_resid_t*      res
                                               );

/* userres.c */
homu_main_result_t homu_main_setUserRes (
                                          struct homu_main_state*     state,
                                          homu_main_userid_t          userid,
                                          homu_main_resid_t           resid,
                                          enum homu_main_userresType  type,
                                          char*                       str
                                        );

/* process.c */
homu_main_prosid_t homu_main_copyProcess (
                                           struct homu_main_state* state,
                                           homu_main_userid_t      userid,
                                           homu_main_prosid_t      origProsId
                                         );
void homu_main_addCopiedProcess (
                                  struct homu_main_state* state,
                                  homu_main_userid_t      userid,
                                  char*                   path,
                                  homu_main_prosid_t      prosid
                                );
homu_main_prosid_t homu_main_addProcess (
                                          struct homu_main_state* state,
                                          homu_main_userid_t      userid,
                                          char*                   path
                                        );
void homu_main_delProcess (
                            struct homu_main_state* state,
                            homu_main_userid_t      userid,
                            homu_main_prosid_t      prosid
                          );

/* mode.c */
enum homu_main_mode homu_main_getMode (
                                        struct homu_main_state* state,
                                        homu_main_userid_t      userid
                                      );
homu_main_result_t  homu_main_setMode (
                                        struct homu_main_state* state,
                                        homu_main_userid_t      userid,
                                        enum homu_main_mode     mode
                                      );

/* info.c */
struct homu_main_info* homu_main_getInfo (
                                           struct homu_main_state* state,
                                           homu_main_userid_t      userid,
                                           struct homu_main_info*  info
                                         );

/* error.c */
void homu_main_throwError (
                            struct homu_main_state* state,
                            homu_main_userid_t      userid,
                            enum homu_main_errType  err 
                          );

/* judge.c */
homu_main_judge_t homu_main_judge (
                                    struct homu_main_state* state,
                                    homu_main_userid_t      userid,
                                    homu_main_prosid_t      prosid,
                                    homu_main_resid_t       resid,
                                    const char*             str
                                  );

/* memory.c */
void* homu_main_malloc (
                         struct homu_main_state*  state,
                         void*                    ptr,
                         homu_main_size_t         size
                       );
void  homu_main_mrelease (
                           struct homu_main_state*  state,
                           void*                    ptr
                         );

/* log.c */

const char* homu_main_readLog (
                                struct homu_main_state* state,
                                homu_main_userid_t      userid
                              );
void homu_main_writeLog (
                          struct homu_main_state* state,
                          homu_main_userid_t      userid,
                          const char*             msg
			 );

/* string.c */
unsigned int      homu_main_str2uint  (const char* str);
int               homu_main_strcmp    (const char* str1, const char* str2);
homu_main_size_t  homu_main_strlen    (const char* str);
char*             homu_main_strcpy    (char* str1, const char* str2);
void*             homu_main_memcpy    (void* dest, const void* src, homu_main_size_t num);


#endif

