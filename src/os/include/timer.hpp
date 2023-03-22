#pragma once

#include "types.hpp"
#include "clock.hpp"

namespace qOS {

    class timer {
        protected:
            clock_t tStart;
            clock_t tv;
        public:
            timer();
            bool set( time_t tTime ); 
            void disarm( void );
            void reload( void );
            bool status( void ) const;
            bool freeRun( time_t tTime );
            clock_t remaining( void ) const;
            clock_t elapsed( void ) const;
            bool expired( void ) const;
            bool deadLineCheck( void ) const;
            clock_t getInterval( void ) const;
            timer& operator=( time_t tTime );
            timer& operator=( bool en );
            bool operator()( void );
            bool operator()( time_t tTime );
            void operator()( bool en );

            static const bool ARMED;
            static const bool DISARMED;
            static const clock_t DISARM_VALUE;
            static const clock_t REMAINING_IN_DISARMED_STATE;
    };

}

bool operator==( const qOS::timer& obj, bool value );

