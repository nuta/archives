
#include "homu_main.h"

/**
*
*  @note
*
*    The allocated size is written at the beginning of allocated memory space.
*
*
*  @warning
*
*    state->malloc() and state->mfree() must *not* return NULL.
*    Errors caused in these functions must be handled in each function.
*
*/



void* homu_main_malloc (
                         struct homu_main_state* state,
                         void*                   ptr,
                         homu_main_size_t        size
                       ){

 void*             newptr;
 void*             newptr_data;
 homu_main_size_t* newptr_size;


  newptr      = state->malloc(size + sizeof(homu_main_size_t));
  newptr_size = newptr;
  newptr_data = ((homu_main_uint8_t *) newptr + sizeof(homu_main_size_t));

  if(ptr != HOMU_MAIN_NULL){

    homu_main_memcpy(
                      newptr_data,
                      ptr,
                      *((homu_main_size_t *) ((homu_main_uint8_t *) ptr - sizeof(homu_main_size_t)))
                    );

    homu_main_mrelease(state, ptr);
  }

  /* write the allocated size */
  *newptr_size = size;
  return newptr_data;
}



void homu_main_mrelease (struct homu_main_state* state, void* ptr){

 homu_main_uint8_t* tmpptr;

  tmpptr = ((homu_main_uint8_t *) ptr - sizeof(homu_main_size_t));
  state->mfree(tmpptr);
  return;
}
