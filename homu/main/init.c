
#include "homu_main.h"



homu_main_result_t homu_main_init (
                                    struct homu_main_state* state,
                                    void* (*malloc) (homu_main_size_t size),
                                    void  (*mfree)  (void* ptr)
                                  ){

  state->malloc  = malloc;
  state->mfree   = mfree;

  return HOMU_MAIN_SUCCESS;
}

