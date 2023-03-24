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
            bool set( qOS::time_t tTime ); 
            void disarm( void );
            void reload( void );
            bool status( void ) const;
            bool freeRun( qOS::time_t tTime );
            qOS::clock_t remaining( void ) const;
            qOS::clock_t elapsed( void ) const;
            bool expired( void ) const;
            bool deadLineCheck( void ) const;
            qOS::clock_t getInterval( void ) const;
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

