#ifndef QOS_CPP_CRITICAL
#define QOS_CPP_CRITICAL

#include "types.hpp"

namespace qOS {

    using int_restorer_t = void (*)( std::uint32_t );
    using int_disabler_t = std::uint32_t (*)( void );

    namespace critical {
        void enter( void );
        void exit( void );
        bool setInterruptsED( const int_restorer_t rFcn, const int_disabler_t dFcn );
    };
}

#endif /*QOS_CPP_CRITICAL*/