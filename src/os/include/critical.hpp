#ifndef QOS_CPP_CRITICAL
#define QOS_CPP_CRITICAL

#include "types.hpp"

namespace qOS {

    using int_restorer_t = void (*)( std::uint32_t );
    using int_disabler_t = std::uint32_t (*)( void );

    namespace critical {
        /**
        * @brief Enter a critical section. This function invokes the @b Disabler
        * function if available.
        * @note  Please see critical::setInterruptsED()
        */
        void enter( void ) noexcept;
        /**
        * @brief Exit a critical section. This function invokes the @b Enabler
        * function if available.
        * @note  Please see critical::setInterruptsED()
        */
        void exit( void ) noexcept;
        /**
        * @brief Set the hardware-specific code for global interrupt enable/disable.
        * Setting this allows you to communicate safely from Interrupts using 
        * queued-notifications or queues.
        * @param[in] rFcn The function with hardware specific code that enables or
        * restores interrupts.
        * @param[in] dFcn The function with hardware specific code that disables
        * interrupts.
        * @return true on success. Otherwise return false.
        */
        bool setInterruptsED( const int_restorer_t rFcn, const int_disabler_t dFcn ) noexcept;
    };
}

#endif /*QOS_CPP_CRITICAL*/
