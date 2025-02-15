//! \file triceStackBuffer.c
//! \author Thomas.Hoehenleitner [at] seerose.net
//! //////////////////////////////////////////////////////////////////////////
#include "trice.h"

#if TRICE_MODE == TRICE_STACK_BUFFER

#if defined(TRICE_STACK_BUFFER_MAX_SIZE) && !defined(TRICE_SINGLE_MAX_SIZE)
#define TRICE_SINGLE_MAX_SIZE (TRICE_STACK_BUFFER_MAX_SIZE - TRICE_DATA_OFFSET)
#endif

#if TRICE_SINGLE_MAX_SIZE + TRICE_DATA_OFFSET > TRICE_STACK_BUFFER_MAX_SIZE
//                                      #define TRICE_STACK_BUFFER_MAX_SIZE TRICE_BUFFER_SIZE 
//                                                                  #define TRICE_BUFFER_SIZE (TRICE_SINGLE_MAX_SIZE + 8)
//      40                     32
#error  
#endif


//! TriceDepthMax returns the max trice buffer depth until now.
size_t TriceDepthMax( void ){
    return triceDepthMax;
}

void TriceLogBufferInfo( void ){
    TRICE32( id( 6945), "att:Single Trice Stack buf size:%4u", TRICE_SINGLE_MAX_SIZE + TRICE_DATA_OFFSET );
}

#endif // #if TRICE_MODE == TRICE_STACK_BUFFER
