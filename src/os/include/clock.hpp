#ifndef QOS_CPP_CLOCK
#define QOS_CPP_CLOCK

#include "types.hpp"
#include "config.h"

namespace qOS {

    using clock_t = std::uint32_t ;

    #if ( Q_SETUP_TIME_CANONICAL == 1 )
        using time_t = std::uint32_t;
    #else
        using time_t = float32_t;
    #endif

    #if ( Q_SETUP_TICK_IN_HERTZ == 1 )
        using timingBase_t = qOS::clock_t;
    #else
        /** @brief A macro to specify a qTime_t type for time-base APIs.*/
        using timingBase_t = qOS::time_t;
    #endif

    using getTickFcn_t = clock_t (*)( void );

    class clock {
        protected:
            clock();
        public:
            clock( clock &other ) = delete;
            void operator=( const clock & ) = delete;
            static getTickFcn_t getTick;
            static qOS::clock_t convert2Clock( const qOS::time_t t ) noexcept;
            static qOS::time_t convert2Time( const qOS::clock_t t ) noexcept;
            static void sysTick( void ) noexcept;
            static bool timeDeadLineCheck( const qOS::clock_t ti, const qOS::clock_t td ) noexcept;
            static bool setTimeBase( const timingBase_t tb ) noexcept;
            static bool setTickProvider( const getTickFcn_t provider ) noexcept;
            static const qOS::time_t NONE;
            static const qOS::time_t IMMEDIATE;
            static const qOS::time_t MINUTE;
            static const qOS::time_t HOUR;
            static const qOS::time_t DAY;
            static const qOS::time_t WEEK;
    };

}

#endif /*QOS_CPP_CLOCK*/