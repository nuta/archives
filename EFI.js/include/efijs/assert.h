#pragma once
#include <efijs/message.h>

#define EFIJS_ASSERT_EQ(e1, e2) do {                    \
         if (e1 != e2) {                                \
             EFIJS_PANIC("Assertion failed: " #e1 "should equals" #e2);  \
         }                                              \
    } while (0)

#define EFIJS_ASSERT_NEQ(e1, e2) do {                   \
         if (e1 == e2) {                                \
             EFIJS_PANIC("Assertion failed: " #e1 "should NOT equals" #e2);  \
         }                                              \
    } while (0)
