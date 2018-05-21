#ifndef __CPP_ASSERT_H__
#define __CPP_ASSERT_H__

#define assert(x)  do { \
       if (!(x)) { \
           WARN("Assertion failed: %s", x); \
       } \
   } while (0)

#endif
