#pragma once

#include "types.hpp"
#include "config.h"

namespace qOS {

    typedef uint32_t clock_t;

    #if ( Q_SETUP_TIME_CANONICAL == 1 )
        typedef uint32_t time_t;
    #else
        typedef float32_t time_t;
    #endif

    #if ( Q_SETUP_TICK_IN_HERTZ == 1 )
        typedef clock_t timingBase_t;
    #else
        /** @brief A macro to specify a qTime_t type for time-base APIs.*/
        typedef time_t timingBase_t;
    #endif

    typedef clock_t (*getTickFcn_t)(void);

    class clock {
        protected:
            clock();
        public:
            clock( clock &other ) = delete;
            void operator=( const clock & ) = delete;
            static getTickFcn_t getTick;
            static clock_t convert2Clock( const time_t t );
            static time_t convert2Time( const clock_t t );
            static void sysTick( void );
            static bool timeDeadLineCheck( const clock_t ti, const clock_t td );
            static bool setTimeBase( const timingBase_t tb );
            static bool setTickProvider( const getTickFcn_t provider );
            static const time_t NONE;
            static const time_t IMMEDIATE;
            static const time_t MINUTE;
            static const time_t HOUR;
            static const time_t DAY;
            static const time_t WEEK;
    };

}
