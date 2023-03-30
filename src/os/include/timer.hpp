#ifndef QOS_CPP_TIMER
#define QOS_CPP_TIMER

#include "types.hpp"
#include "clock.hpp"

namespace qOS {

    class timer {
        protected:
            qOS::clock_t tStart;
            qOS::clock_t tv;
        public:
            timer();
            bool set( qOS::time_t tTime ) noexcept;
            void disarm( void ) noexcept;
            void reload( void ) noexcept;
            bool status( void ) const noexcept;
            bool freeRun( qOS::time_t tTime ) noexcept;
            qOS::clock_t remaining( void ) const noexcept;
            qOS::clock_t elapsed( void ) const noexcept;
            bool expired( void ) const noexcept;
            bool deadLineCheck( void ) const noexcept;
            qOS::clock_t getInterval( void ) const noexcept;
            timer& operator=( qOS::time_t tTime );
            timer& operator=( bool en );
            bool operator()( void );
            bool operator()( qOS::time_t tTime );
            void operator()( bool en );

            static const bool ARMED;
            static const bool DISARMED;
            static const qOS::clock_t DISARM_VALUE;
            static const qOS::clock_t REMAINING_IN_DISARMED_STATE;
    };

}

bool operator==( const qOS::timer& obj, bool value );

#endif /*QOS_CPP_TIMER*/

