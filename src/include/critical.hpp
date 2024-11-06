#ifndef QOS_CPP_CRITICAL
#define QOS_CPP_CRITICAL

#include "include/types.hpp"

namespace qOS {

    /** @addtogroup qcritical
    * @brief APIs to handle entry/exit actions in @ref q_critical.
    *  @{
    */

    /**
    * @brief Function called by critical::exit() to restore interrupts.
    *
    * @note User should use bare-metal code to implement this function.
    * @note The kernel passes the previously saved interrupt configuration as
    * input argument. The user is responsible for taking this argument to
    * restore the saved interrupt setup.
    * Example :
    * @code{.c}
    * void BSP_InterruptRestorer( uint32_t savedMask ) {
    *       HAL_InterruptSetMask( savedMask );
    *       HAL_EnableInterrupts();
    * }
    * @endcode
    * @param[in] savedMask The interrupt configuration saved by the "Disabler"
    * function
    */
    using int_restorer_t = void (*)( uint32_t );

    /**
    * @brief Function called by critical::enter() to disable interrupts.
    *
    * @note User should use bare-metal code to implement this function.
    * @note User should return the current interrupt configuration to be saved
    * by the kernel.
    * Example :
    * @code{.c}
    * uint32_t BSP_InterruptDisabler( void ) {
    *       uint32_t currentMask;
    *       currentMask = HAL_InterruptGetMask( savedMask );
    *       HAL_DisableInterrupts();
    *       return currentMask;
    * }
    * @endcode
    * @return The current interrupt configuration (mask). The kernel will retain
    * this value
    * until the critical section is exited
    */
    using int_disabler_t = uint32_t (*)( void );

    /**
    * @brief OS Critical interfaces.
    */
    namespace critical {

        /** @addtogroup qcritical
        * @brief APIs to handle entry/exit actions in @ref q_critical.
        *  @{
        */

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
        * @return @c true on success. Otherwise return @c false.
        */
        bool setInterruptsED( const int_restorer_t rFcn, const int_disabler_t dFcn ) noexcept;

        /** @}*/
    }

    /** @}*/
}

#endif /*QOS_CPP_CRITICAL*/
