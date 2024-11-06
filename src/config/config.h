/*!
 * @file config.h
 * @author J. Camilo Gomez C.
 * @version 1.10
 * @note This file is part of the QuarkTS distribution.
 * @brief OS Configuration file
 **/

#ifndef QCONFIG_H
    #define QCONFIG_H

    /** @addtogroup  qconfiguration
     * @brief  Some OS features can be customized using a set of macros located
     * in the header file @c config.h.
    *  @{
    */

    /*==================================================  CONFIGURATION FLAGS  =======================================================*/
    /**
    * @brief Number of priority levels.
    * @details The number of priorities available to the application tasks
    * @note Default value @c 3
    */
    #define Q_PRIORITY_LEVELS           ( 3 )       /**< The number of priorities available to the application tasks*/
    /**
    * @brief The size of the priority queue.
    * @details The size of the priority FIFO queue for notifications (use a 0(zero) value to disable it)
    * @note Default value @c 10
    */
    #define Q_PRIO_QUEUE_SIZE           ( 10 )
    /**
    * @brief Allow scheduler release action.
    * @details Used to enable or disable the release of the scheduling.
    * @note Default value @c 1 @a enabled
    */
    #define Q_ALLOW_SCHEDULER_RELEASE   ( 1 )
    /**
    * @brief Preserve task entry order.
    * @details If enabled, kernel will preserve the tasks entry order every OS scheduling cycle.
    * @note Default value @c 1 @a enabled
    */
    #define Q_PRESERVE_TASK_ENTRY_ORDER ( 1 )
    /**
    * @brief Byte alignment used by the memory manager.
    * @details Use a number in power of two for this configuration.
    * @note Default value @c 8
    */
    #define Q_BYTE_ALIGNMENT            ( 8 )
    /**
    * @brief Heap size fo the default memory pool.
    * @details This also will enable @c new and @c delete operators if not available
    * in the target compiler.
    * @note Default value @c 0 @a disabled
    */
    #define Q_DEFAULT_HEAP_SIZE         ( 0 )
    /**
    * @brief Finite State Machine enabler.
    * @details Used to enable or disable the Finite State Machine (FSM) module
    * @note Default value @c 1 @a enabled
    */
    #define Q_FSM                       ( 1 )
    /**
    * @brief Finite State Machine : nest depth
    * @details The max depth of nesting for the Finite State Machines (FSM) module.
    * This setting its required to allow hierarchical state machines. Use a value
    * greater than 1
    * @note Default value @c 5
    */
    #define Q_FSM_MAX_NEST_DEPTH        ( 5 )
    /**
    * @brief Finite State Machine : number of timeouts.
    * @details Max number of timeouts inside a timeout specification for the
    * Finite State machine (FSM) module
    * @note Default value @c 3
    */
    #define Q_FSM_MAX_TIMEOUTS          ( 3 )
    /**
    * @brief Finite State Machine : number of signals to subscribe.
    * @details Max number of signals to subscribe for a single FSM object.
    * @note Default value @c 8
    */
    #define Q_FSM_PS_SIGNALS_MAX        ( 8 )
    /**
    * @brief Finite State Machine : subscribers per signal.
    * @details  Max number of FSM subscribers per signal.
    * @note Default value @c 4
    */
    #define Q_FSM_PS_SUB_PER_SIGNAL_MAX ( 4 )
    /**
    * @brief Size for the trace internal buffer
    * @details The trace object requires an internal buffer for standard integer
    * conversions. This macro defines its size. 36 bytes should be enough
    * @note Default value @c 36
    */
    #define Q_TRACE_BUFSIZE             ( 36 )
    /**
    * @brief AT Command Line Interface enabler
    * @details Used to enable or disable the AT Command Line Interface(CLI)
    * @note Default value @c 1 @a enabled
    */
    #define Q_CLI                       ( 1 )
    /**
    * @brief Queues enabler
    * @details Used to enable or disable OS queues
    * @note Default value @c 1 @a enabled
    */
    #define Q_QUEUES                    ( 1 )
    /**
    * @brief Colored output in logger
    * @details Used to enable colored output in supported terminals when using
    * logger
    * @note Default value @c 0 @a disabled
    */
    #define Q_LOGGER_COLORED            ( 0 )
    /*================================================================================================================================*/

    /** @}*/
#endif
