#ifndef QOS_CPP_TIMER
#define QOS_CPP_TIMER

#include "include/types.hpp"
#include "include/clock.hpp"

namespace qOS {

    /** @addtogroup qstimers Timers
    * @brief API interface for the @ref q_stimers extension.
    * @pre This extension requires the operating system is previously initialized
    * with core::init() with a functional time-base. Please read @ref q_timmingapproach .
    *  @{
    */

    /**
    * @brief A non-blocking Timer object
    * @details Timers are an essential extension as they allow for accurate and
    * efficient timekeeping without blocking tasks. Using timers enables the 
    * embedded application to perform other critical tasks while the timer is 
    * running in the background. Timers also provide flexibility in the event
    * that the timer needs to be paused, restarted, or adjusted on the fly.
    * This can be particularly useful in applications that require dynamic 
    * timing or have unpredictable event intervals.
    */
    class timer {
        private:
            qOS::clock_t tStart{ 0u };
            qOS::clock_t tv{ 0u };
        public:
            timer();
            /**
            * @brief Initializes the instance of the timer with the specified
            * expiration time. Timer will start armed.
            * @note The OS must be running before using timers.
            * @note The expiration time should be at least, two times greater than
            * the clock-Tick.
            * @param[in] tTime The expiration time.
            */
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
            * @return Returns @c true on success, otherwise, returns @c false.
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
            /**
            * @brief Get the current status of the Timer (Armed or Disarmed)
            * @return @c true when armed, otherwise @c false when disarmed.
            */
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
            * @note The OS must be running before using a timer.
            * @note The expiration time should be at least, two times greater than
            * the clock-Tick.
            * @param[in] tTime The expiration time.
            * @return Returns @c true on success, otherwise, returns @c false.
            * @note A disarmed timer also returns @c false.
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
            /**
            * @brief Check if the specified deadline for has been reached.
            * @return @c true if the deadline has been reached, otherwise, returns @c false.
            */
            bool deadLineCheck( void ) const noexcept;
            /**
            * @brief Retrieve the timer interval value
            * @return The interval value in epochs.
            */
            qOS::clock_t getInterval( void ) const noexcept;
            /**
            * @brief Set the expiration time for a timer and gets armed immediately
            * @note The OS must be running before using timers.
            * @note The expiration time should be at least, two times greater than
            * the clock-Tick.
            * @param[in] tTime The expiration time.
            */
            timer& operator=( qOS::time_t tTime );
            /**
            * @brief Disarms of reload the timer
            * @note The OS must be running before using timers.
            * @note The expiration time should be at least, two times greater than
            * the clock-Tick.
            * @param[in] en @c true for reload of @c false to disarm.
            */
            timer& operator=( bool en );
            /**
            * @brief Non-Blocking timer check
            * @return Returns true when timer expires, otherwise, returns false.
            * @note A disarmed timer also returns false.
            */
            bool operator()( void );
            /**
            * @brief Set the expiration time for a timer and gets armed immediately
            * @note The OS must be running before using timers.
            * @note The expiration time should be at least, two times greater than
            * the clock-Tick.
            * @param[in] tTime The expiration time.
            */
            bool operator()( qOS::time_t tTime );
            /**
            * @brief Disarms of reload the timer
            * @note The OS must be running before using timers.
            * @note The expiration time should be at least, two times greater than
            * the clock-Tick.
            * @param[in] en @c true for reload of @c false to disarm.
            */
            void operator()( bool en );
            /** @brief Constant that defines the status of an armed timer*/
            static const bool ARMED;
            /** @brief Constant that defines the status of a disarmed timer*/
            static const bool DISARMED;
            /** @brief Constant that defines the value of a disarmed timer*/
            static const qOS::clock_t DISARM_VALUE;
            /** @brief Constant that defines the value that a disarmed timer 
            * returns when the remaining time is requested.
            */
            static const qOS::clock_t REMAINING_IN_DISARMED_STATE;
    };

    /** @}*/
}

bool operator==( const qOS::timer& obj, bool value );

#endif /*QOS_CPP_TIMER*/

