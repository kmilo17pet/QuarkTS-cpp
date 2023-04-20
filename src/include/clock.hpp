#ifndef QOS_CPP_CLOCK
#define QOS_CPP_CLOCK

#include "include/types.hpp"
#include "config/config.h"

namespace qOS {

    /** @addtogroup qclock
    * @brief Clock and time manipulation interface used by the entire OS.
    * @{
    */

    using clock_t = timeCount_t;

    #if ( Q_SETUP_TIME_CANONICAL == 1 )
        using time_t = timeCount_t;
    #else
        using time_t = float32_t;
    #endif

    #if ( Q_SETUP_TICK_IN_HERTZ == 1 )
        using timingBase_t = qOS::clock_t;
    #else
        using timingBase_t = qOS::time_t;
    #endif

    using getTickFcn_t = clock_t (*)( void );

    /**
    * @brief A class to encapsulate the OS clock
    */
    class clock final {
        protected:
            static timingBase_t timingBase;
            static volatile qOS::clock_t sysTick_Epochs;
            static qOS::clock_t internalTick( void ) noexcept;
            clock();
        public:
            clock( clock &other ) = delete;
            void operator=( const clock & ) = delete;
            /**
            * @brief Return the current tick used by the OS
            * @return time (t) in epochs.
            */
            static getTickFcn_t getTick; /* skipcq: CXX-W2009 */
            /**
            * @brief Convert the specified input time to clock epochs
            * @param[in] t Time input
            * @return time (t) in epochs
            */
            static qOS::clock_t convert2Clock( const qOS::time_t t ) noexcept;
            /**
            * @brief Convert the specified input clock epochs to time
            * @param[in] t Time in epochs
            * @return time (t)
            */
            static qOS::time_t convert2Time( const qOS::clock_t t ) noexcept;
            /**
            * @brief Feed the system tick.
            * @note This call is mandatory and must be called once inside the
            * dedicated timer interrupt service routine (ISR). Example
            * @code{.c}
            * void interrupt xPeriodicTimer_ISR( void ) {
            *       qOS::clock::sysTick();
            * }
            * @endcode
            */
            static void sysTick( void ) noexcept;
            /**
            * @brief Perform a timestamp check. This function computes the amount
            * of time elapsed between the current instant and the init timestamp @a ti
            * and checks if the result is greater than @a td.
            * @param[in] ti Init timestamp in epochs
            * @param[in] td Elapsed time to check in epochs
            * @return true if the elapsed time (t-ti) is greater or equal to td.
            * Otherwise returns false
            */
            static bool timeDeadLineCheck( const qOS::clock_t ti, const qOS::clock_t td ) noexcept;
            /**
            * @brief Set the system time-base for time conversions
            * @param[in]  tb Time base
            * @return true on success, otherwise returns false.
            */
            static bool setTimeBase( const timingBase_t tb ) noexcept;
            /**
            * @brief Set the clock-tick provider function.
            * @param[in] provider A pointer to the tick provider function
            * @return true on success, otherwise returns false.
            */
            static bool setTickProvider( const getTickFcn_t provider ) noexcept;
            /** @brief To specify a null time value.*/
            static const qOS::time_t NONE;
            /** @brief To specify a non-wait time value.*/
            static const qOS::time_t IMMEDIATE;
    };

    /** @}*/
}



#endif /*QOS_CPP_CLOCK*/
