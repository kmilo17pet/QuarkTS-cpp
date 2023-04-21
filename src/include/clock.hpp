#ifndef QOS_CPP_CLOCK
#define QOS_CPP_CLOCK

#include "include/types.hpp"
#include "config/config.h"

namespace qOS {

    /** @addtogroup qclock
    * @brief Clock and time manipulation interface used by the entire OS.
    * @{
    */

   /** @brief A unsigned integer to hold ticks count. Epochs counter.*/
    using clock_t = timeCount_t;

    #if ( Q_SETUP_TIME_CANONICAL == 1 )
        /** @brief The typedef that specified an time quantity, usually expressed in milliseconds.*/
        using time_t = timeCount_t;
    #else
        /** @brief The typedef that specified an time quantity, usually expressed in seconds.*/
        using time_t = float32_t;
    #endif

    #if ( Q_SETUP_TICK_IN_HERTZ == 1 )
        /** @brief A type to specify a clock_t type for time-base APIs.*/
        using timingBase_t = qOS::clock_t;
    #else
        /** @brief A type to specify a time_t type for time-base APIs.*/
        using timingBase_t = qOS::time_t;
    #endif

    /**
    * @brief Pointer to a function that gets the current hardware tick value.
    *
    * @note User should use bare-metal code to implement this function.
    * Example :
    * @code{.c}
    * unsigned long OSInterface_GetTick( void ) {
    *       return HAL_GetTick();
    * }
    * @endcode
    * @return The number of ticks provided by the system HAL.
    */
    using getTickFcn_t = clock_t (*)( void );

    /**
    * @brief A class to encapsulate the OS clock
    */
    class clock final {
        protected:
            /*! @cond  */
            static timingBase_t timingBase;
            static volatile qOS::clock_t sysTick_Epochs;
            static qOS::clock_t internalTick( void ) noexcept;
            clock();
            /*! @endcond  */
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
            * @return @c true if the elapsed time (t-ti) is greater or equal to td.
            * Otherwise returns @c false
            */
            static bool timeDeadLineCheck( const qOS::clock_t ti, const qOS::clock_t td ) noexcept;
            /**
            * @brief Set the system time-base for time conversions
            * @param[in]  tb Time base
            * @return @c true on success, otherwise returns @c false.
            */
            static bool setTimeBase( const timingBase_t tb ) noexcept;
            /**
            * @brief Set the clock-tick provider function.
            * @param[in] provider A pointer to the tick provider function
            * @return @c true on success, otherwise returns @c false.
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
