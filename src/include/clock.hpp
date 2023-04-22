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

    /** @brief The typedef that specified an time quantity, usually expressed in milliseconds.*/
    using time_t = timeCount_t;

    /*! @cond  */
    /*cstat -CERT-FLP34-C -MISRAC++2008-5-0-5 -MISRAC++2008-5-0-7*/
    constexpr qOS::time_t operator "" _ms( unsigned long long int x )
    {
        return static_cast<qOS::time_t>( x );
    }
    constexpr qOS::time_t operator "" _sec( unsigned long long int x )
    {
        return static_cast<qOS::time_t>( 1000u*x );
    }
    constexpr qOS::time_t operator "" _sec( long double x )
    {
        return static_cast<qOS::time_t>( 1000u*x );
    }
    constexpr qOS::time_t operator "" _minutes( unsigned long long int x )
    {
        return static_cast<qOS::time_t>( 60000u*x );
    }
    constexpr qOS::time_t operator "" _minutes( long double x )
    {
        return static_cast<qOS::time_t>( 60000u*x );
    }
    constexpr qOS::time_t operator "" _hours( unsigned long long int x )
    {
        return static_cast<qOS::time_t>( 3600000u*x );
    }
    constexpr qOS::time_t operator "" _hours( long double x )
    {
        return static_cast<qOS::time_t>( 3600000u*x );
    }
    constexpr qOS::time_t operator "" _days( unsigned long long int x )
    {
        return static_cast<qOS::time_t>( 86400000u*x );
    }
    constexpr qOS::time_t operator "" _days( long double x )
    {
        return static_cast<qOS::time_t>( 86400000*x );
    }
    /*cstat +CERT-FLP34-C +MISRAC++2008-5-0-5 +MISRAC++2008-5-0-7*/
    /*! @endcond  */


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
            static volatile qOS::clock_t sysTick_Epochs; // skipcq: CXX-W2009
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
            static getTickFcn_t getTick; // skipcq: CXX-W2009
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
