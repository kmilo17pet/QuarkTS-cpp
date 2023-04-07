#ifndef QOS_CPP_TIMER
#define QOS_CPP_TIMER

#include "types.hpp"
#include "clock.hpp"

namespace qOS {

    /**
    * @brief A non-blocking Timer object
    */
    class timer {
        private:
            qOS::clock_t tStart{ 0u };
            qOS::clock_t tv{ 0u };
        public:
            timer();
            timer( qOS::time_t tTime )
            {
                (void)set( tTime );
            }
            /**
            * @brief Set the expiration time for a timer. On success, the timer gets
            * armed immediately
            * @note The OS must be running before using timers.
            * @note The expiration time should be at least, two times greater than
            * the clock-Tick.
            * @param[in] tTime The expiration time.
            * @return Returns true on success, otherwise, returns false.
            */
            bool set( qOS::time_t tTime ) noexcept;
            /**
            * @brief Disarms the timer object
            */
            void disarm( void ) noexcept;
            /**
            * @brief Reload the timer with the previous specified time.
            * @note Timer should be armed before this operation
            */
            void reload( void ) noexcept;
            bool status( void ) const noexcept;
            /**
            * @brief Non-Blocking timer check with automatic arming.
            *
            * Behavior:
            * If disarmed, it gets armed immediately with the specified time.
            *
            * If armed, the time argument is ignored and the API only checks for
            * expiration. When the time expires, the STimer gets armed immediately
            * taking the specified time.
            * @note After the timer expiration, this method re-arms the timer
            * @note The OS must be running before using STimers.
            * @note The expiration time should be at least, two times greater than
            * the clock-Tick.
            * @param[in] tTime The expiration time(Must be specified in seconds).
            * @return Returns true on success, otherwise, returns false.
            * @note A disarmed timer also returns false.
            */
            bool freeRun( qOS::time_t tTime ) noexcept;
            /**
            * @brief Retrieve the remaining time in epochs
            * @return The remaining time specified in epochs.
            */
            qOS::clock_t remaining( void ) const noexcept;
            /**
            * @brief Retrieve the elapsed time in epochs
            * @return The Elapsed time specified in epochs.
            */
            qOS::clock_t elapsed( void ) const noexcept;
            /**
            * @brief Non-Blocking timer check
            * @return Returns true when timer expires, otherwise, returns false.
            * @note A disarmed timer also returns false.
            */
            bool expired( void ) const noexcept;
            bool deadLineCheck( void ) const noexcept;
            qOS::clock_t getInterval( void ) const noexcept;
            timer& operator=( qOS::time_t tTime );
            timer& operator=( bool en );
            bool operator()( void );
            bool operator()( qOS::time_t tTime );
            void operator()( bool en );
            /** @brief Constant that defines the status of an armed STimer*/
            static const bool ARMED;
            /** @brief Constant that defines the status of a disarmed STimer*/
            static const bool DISARMED;
            /** @brief Constant that defines the value of a disarmed STimer*/
            static const qOS::clock_t DISARM_VALUE;
            /** @brief Constant that defines the value that a disarmed STimer 
            * returns when the remaining time is requested.
            */
            static const qOS::clock_t REMAINING_IN_DISARMED_STATE;
    };

}

bool operator==( const qOS::timer& obj, bool value );

#endif /*QOS_CPP_TIMER*/

