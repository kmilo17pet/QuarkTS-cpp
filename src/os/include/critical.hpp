#pragma once

#include "types.hpp"

namespace qOS {
    typedef void (*int_restorer_t)( uint32_t arg1 );
    typedef uint32_t (*int_disabler_t)( void );

    namespace critical {
        void enter( void );
        void exit( void );
        bool setInterruptsED( const int_restorer_t rFcn, const int_disabler_t dFcn );
    };
}