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
        * @brief Enables a scoped critical section with minimal syntax overhead.
        *
        * @code{.cpp}
        * void example() {
        *     // code here : normal section(non-critical)
        *     critical::scope {
        *         // Code here runs in a critical section
        *     }
        *     // code here, normal section (non-critical)
        * }
        * @endcode
        */
        inline void scope( void ) noexcept {}

        /**
        * @brief Scoped critical section lock.
        *
        * @details
        * The `lock` class provides a simple RAII-based mechanism for managing critical
        * sections. When an object of this class is created, it automatically enters a
        * critical section (disabling interrupts). When the object is destroyed, it exits
        * the critical section (restoring interrupts).
        *
        * @code{.cpp}
        * void example() {
        *     critical::lock l
        *     // Code that runs in a critical section
        * }
        * @endcode
        */
        class lock final : private nonCopyable {
            private:
                static void enter( void ) noexcept;
                static void exit( void ) noexcept;
                static int_disabler_t disable;
                static int_restorer_t restore;
                static volatile uint32_t flags;
                static volatile int nestingLevel;
                bool entered{ true };
                friend bool setInterruptsED( const int_restorer_t rFcn, const int_disabler_t dFcn ) noexcept;
            public:
                lock() noexcept;
                ~lock() noexcept;
                explicit operator bool() noexcept;
        };

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

/*! @cond  */
/*============================================================================*/
#define scope                                                                  \
scope();                                                                       \
for ( critical::lock critical_lock; critical_lock; )                           \
/*============================================================================*/
/*! @endcond  */

#endif /*QOS_CPP_CRITICAL*/
