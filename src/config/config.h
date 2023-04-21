/*!
 * @file config.h
 * @author J. Camilo Gomez C.
 * @version 1.10
 * @note This file is part of the QuarkTS distribution.
 * @brief OS Configuration file
 **/

#ifndef QCONFIG_H
    #define QCONFIG_H

    /*==================================================  CONFIGURATION FLAGS  =======================================================*/
    #define Q_PRIORITY_LEVELS           ( 3 )       /**< The number of priorities available to the application tasks*/
    #define Q_SETUP_TIME_CANONICAL      ( 1 )       /**< If enabled, kernel assumes the timing Base to 1mS(1KHz). All time specifications for tasks and STimers must be set in mS*/
    #define Q_SETUP_TICK_IN_HERTZ       ( 0 )       /**< If enabled, the timing base will be taken as frequency(Hz) instead of period(S)*/
    #define Q_PRIO_QUEUE_SIZE           ( 10 )      /**< The size of the priority queue (use a 0(zero) value to disable it)*/
    #define Q_ALLOW_SCHEDULER_RELEASE   ( 1 )       /**< Used to enable or disable the release of the scheduling */
    #define Q_PRESERVE_TASK_ENTRY_ORDER ( 1 )       /**< If enabled, kernel will preserve the tasks entry order every OS scheduling cycle*/
    #define Q_BYTE_ALIGNMENT            ( 8 )       /**< Byte alignment used by the memory manager*/
    #define Q_DEFAULT_HEAP_SIZE         ( 0 )       /**< The default heap size for the memory manager. This will enable @c new and @c delete operators if not available*/
    #define Q_FSM                       ( 1 )       /**< Used to enable or disable the Finite State Machine (FSM) module */
    #define Q_FSM_MAX_NEST_DEPTH        ( 5 )       /**< The max depth of nesting for the Finite State Machines (FSM) module*/
    #define Q_FSM_MAX_TIMEOUTS          ( 3 )       /**< Max number of timeouts inside a timeout specification for the Finite State machine (FSM) module*/
    #define Q_FSM_PS_SIGNALS_MAX        ( 8 )       /**< Max number of signals to subscribe for a Finite State Machine (FSM)*/
    #define Q_FSM_PS_SUB_PER_SIGNAL_MAX ( 4 )       /**< Max number of FSM subscribers per signal*/
    #define Q_TRACE_BUFSIZE             ( 36 )      /**< Size for the debug/trace buffer: 36 bytes should be enough*/
    #define Q_CLI                       ( 1 )       /**< Used to enable or disable the AT Command Line Interface(CLI)*/
    #define Q_QUEUES                    ( 1 )       /**< Used to enable or disable queues*/
    #define Q_ATOF_FULL                 ( 1 )       /**< Used to enable or disable the extended "e" notation parsing in qAtoF*/

    /*================================================================================================================================*/  
#endif
