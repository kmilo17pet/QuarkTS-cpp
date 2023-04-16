#ifndef QOS_CPP_KERNEL
#define QOS_CPP_KERNEL

#include "config/config.h"
#include "include/list.hpp"
#include "include/clock.hpp"
#include "include/types.hpp"
#include "include/task.hpp"
#include "include/prioqueue.hpp"

#if ( Q_FSM == 1 )
    #include "fsm.hpp"
#endif
#if ( Q_CLI == 1 )
    #include "cli.hpp"
#endif

namespace qOS {

    using coreFlags_t = uint32_t ;

    /**
    * @brief An enum that defines the modes in which a notification can be
    * delivered
    */
    enum class notifyMode : uint8_t {
        SIMPLE = 0u,    /**< To notify a task using the simple approach. */
        QUEUED = 1u,    /**< To notify a task using the FIFO priority queue. */
        _NONE_ = 3u,    /**< Do not use this value. Used only internally.*/
    };
    struct _notificationSpreader_s {
        notifyMode mode;
        void *eventData;
    };
    using notificationSpreader_t = struct _notificationSpreader_s;

    constexpr taskFlag_t EVENT_FLAG( index_t i )
    {
        return ( ( i >= 1u ) && ( i <= 20u ) ) ?  0x00001000uL << ( i - 1u ) : 0x00001000uL;
    }

    class core final : protected _Event {
        private:
            task idle;
            taskFcn_t releaseSchedCallback{ nullptr };
            task *yieldTask{ nullptr };
            pq::queueStack_t pq_stack[ Q_PRIO_QUEUE_SIZE ];
            prioQueue priorityQueue{ pq_stack, sizeof(pq_stack)/sizeof(pq::queueStack_t) };
            volatile coreFlags_t flag{ 0uL };
            notificationSpreader_t nSpreader{ notifyMode::_NONE_, nullptr };
            size_t taskEntries{ 0uL };
            list coreLists[ Q_PRIORITY_LEVELS + 2 ];
            list& waitingList{ coreLists[ Q_PRIORITY_LEVELS ] };
            list& suspendedList{ coreLists[ Q_PRIORITY_LEVELS + 1 ] };
            list& readyList{ coreLists[ 0 ] };
            const priority_t MAX_PRIORITY_VALUE = static_cast<priority_t>( Q_PRIORITY_LEVELS ) - 1u;
            const uint32_t BIT_INIT = 0x00000001uL;
            const uint32_t BIT_FCALL_IDLE = 0x00000002uL;
            const uint32_t BIT_RELEASE_SCHED = 0x00000004uL;
            const uint32_t BIT_FCALL_RELEASED = 0x00000008uL;
            void triggerReleaseSchedEvent( void ) noexcept;
            bool checkIfReady( void ) noexcept;
            void dispatchTaskFillEventInfo( task *Task ) noexcept;
            void dispatch( list * const xList ) noexcept;
            void dispatchIdle( void ) noexcept;
            core() = default;
            core( core &other ) = delete;
            void operator=( const core & ) = delete;
        public:
            /** @brief A constant that holds the value of the lowest priority.*/
            static const priority_t LOWEST_PRIORITY;
            /** @brief A constant that holds the value of the medium priority.*/
            static const priority_t MEDIUM_PRIORITY;
            /** @brief A constant that holds the value of the highest priority.*/
            static const priority_t HIGHEST_PRIORITY;
            /**
            * @brief Returns a reference to the OS control instance
            * @return The OS control instance.
            */
            static core& getInstance( void ) noexcept;
            /**
            * @brief Task Scheduler initialization. This core method is required
            * and must be called once in the application main thread before any 
            * task is being added to the OS.
            * @param[in] tFcn The function that provides the tick value. If the user
            * application uses the qOS::clock::sysTick() from the ISR, this 
            * parameter can be @c nullptr.
            * @note Function should take void and return a 32bit value.
            * @param[in] t (Optional) This parameter specifies the ISR background
            * timer base-time. This can be the period in seconds(Floating-point
            * format) or frequency in Herzt(if @c Q_SETUP_TICK_IN_HERTZ is enabled).
            * @param[in] callbackIdle  Callback function to the Idle Task. To
            * disable the Idle-Task activities, ignore this parameter of pass 
            * @c nullptr as argument.
            * @return true on success. Otherwise return false.
            *
            * Example : When tick is already provided
            * @code{.c}
            * #include "quarkts++.h"
            * #include "HAL.h"
            *
            * #define TIMER_TICK   ( 0.001f )
            *
            * using namespace qOS;
            *
            * void main( void ) {
            *     HAL_Init();
            *     os.init( HAL_GetTick, TIMER_TICK, IdleTask_Callback );
            * }
            * @endcode
            *
            * Example : When the tick is not provided
            * @code{.c}
            * #include "quarkts++.h"
            * #include "DeviceHeader.h"
            *
            * #define TIMER_TICK   ( 0.001f )
            *
            * using namespace qOS;
            *
            * void Interrupt_Timer0( void ) {
            *     clock::sysTick();
            * }
            *
            * void main( void ) {
            *     MCU_Init();
            *     BSP_Setup_Timer0();
            *     os.init( nullptr, TIMER_TICK, IdleTask_Callback );
            *
            * }
            * @endcode
            */
            void init( const getTickFcn_t tFcn, const timingBase_t t, taskFcn_t callbackIdle = nullptr ) noexcept;
            /**
            * @brief Task Scheduler initialization. This core method is required
            * and must be called once in the application main thread before any 
            * task is being added to the OS.
            * @param[in] tFcn The function that provides the tick value. If the user
            * application uses the qOS::clock::sysTick() from the ISR, this 
            * parameter can be @c nullptr.
            * @note Function should take void and return a 32bit value.
            * @param[in] callbackIdle  Callback function to the Idle Task. To
            * disable the Idle-Task activities, ignore this parameter of pass 
            * @c nullptr as argument.
            * @return true on success. Otherwise return false.
            *
            * Example : When tick is already provided
            * @code{.c}
            * #include "quarkts++.h"
            * #include "HAL.h"
            *
            * using namespace qOS;
            *
            * void main( void ) {
            *     HAL_Init();
            *     os.init( HAL_GetTick, IdleTask_Callback );
            * }
            * @endcode
            *
            * Example : When the tick is not provided
            * @code{.c}
            * #include "quarkts++.h"
            * #include "DeviceHeader.h"
            *
            * using namespace qOS;
            *
            * void Interrupt_Timer0( void ) {
            *     clock::sysTick();
            * }
            *
            * void main( void ) {
            *     MCU_Init();
            *     BSP_Setup_Timer0();
            *     os.init( nullptr, IdleTask_Callback );
            *
            * }
            * @endcode
            */
            inline void init( const getTickFcn_t tFcn, taskFcn_t callbackIdle = nullptr ) noexcept
            {
                init( tFcn, 1u, callbackIdle );
            }
            /**
            * @brief Add a task to the scheduling scheme. The task is scheduled to run
            * every @a t time units, @a n times and executing @a callback method on
            * every pass.
            * @param[in] Task The task node.
            * @param[in] callbackFcn A pointer to a void callback method with a qOS::event_t
            * parameter as input argument.
            * @param[in] p Task priority Value. [0(min) - @c Q_PRIORITY_LEVELS(max)]
            * @param[in] t Execution interval (time).
            * For immediate execution use t = clock::IMMEDIATE.
            * @param[in] n Number of task executions (Integer value). For indefinite
            * execution ( @a n = #qPeriodic or #qIndefinite ). Tasks do not
            * remember the number of iteration set initially. After the
            * iterations are done, internal iteration counter is 0. To perform
            * another set of iterations, set the number of iterations again.
            * @note Tasks which performed all their iterations put their own state to
            * #qDisabled.
            * @note Asynchronous triggers do not affect the iteration counter.
            * @param[in] init Specifies the initial operational state of the task
            * (#qEnabled, #qDisabled, #qAsleep or #qAwake(implies #qEnabled)).
            * @param[in] arg Represents the task arguments. All arguments must be passed
            * by reference and cast to @c void* . Only one argument is allowed, so, for
            * multiple arguments, create a structure that contains all of the arguments
            * and pass a pointer to that structure.
            * @return Returns true on success, otherwise returns false.
            */
            bool addTask( task &Task, taskFcn_t callback, const priority_t p, const time_t t, const iteration_t n, const taskState s = taskState::ENABLED, void *arg = nullptr ) noexcept;
            /**
            * @brief Add a task to the scheduling scheme. This API creates a task with
            * a taskState::DISABLED state by default, so this task will be executed only, when
            * asynchronous events occurs. However, this behavior can be changed in
            * execution time using task::setTime() or task::setIterations().
            * @param[in] Task The task node.
            * @param[in] callbackFcn A pointer to a the task callback method with a
            * event_t parameter as input argument.
            * @param[in] p Task priority Value. [0(min) - @c Q_PRIORITY_LEVELS (max)]
            * @param[in] arg Represents the task arguments. All arguments must be passed
            * by reference and cast to @c void*. 
            * @return Returns true on success, otherwise returns false.
            */
            inline bool addEventTask( task &Task, taskFcn_t callback, const priority_t p, void *arg = nullptr ) noexcept
            {
                return addTask( Task, callback, p, clock::IMMEDIATE, task::SINGLE_SHOT, taskState::DISABLED, arg );
            }
            #if ( Q_FSM == 1 )
            /**
            * @brief Add a task to the scheduling scheme running a dedicated
            * state-machine. The task is scheduled to run every @a t time units in
            * #qPeriodic mode. The event info will be available as a generic pointer
            * inside the sm::handler_t::Data field.
            * @pre The State-machine object should be previously configured with 
            * qStateMachine_Setup()
            * @see qStateMachine_Setup()
            * @param[in] Task  A pointer to the task node.
            * @param[in] m  A pointer to the Finite State-Machine (FSM) object.
            * @param[in] p Task priority Value. [0(min) - @c Q_PRIORITY_LEVELS (max)]
            * @param[in] t Execution time interval. For immediate execution 
            * (tValue = #qTimeImmediate).
            * @param[in] init Specifies the initial operational state of the task
            * (#qEnabled, #qDisabled, #qAsleep or #qAwake(implies #qEnabled)).
            * @param[in] arg Represents the task arguments. All arguments must be
            * passed by reference and cast to @c void*.
            * @return Returns #qTrue on success, otherwise returns #qFalse.
            */
            bool addStateMachineTask( task &Task, stateMachine &m, const priority_t p, const time_t t, const taskState s = taskState::ENABLED, void *arg = nullptr ) noexcept;
            #endif
            #if ( Q_CLI == 1 )
            bool addCommandLineInterfaceTask( task &Task, commandLineInterface &cli, const priority_t p, void *arg = nullptr ) noexcept;
            #endif
            /**
            * @brief Set/Change the callback for the Idle-task
            * @param[in] callbackFcn A pointer to a void callback method with a qOS::event_t
            * parameter as input argument. To disable pass @c nullptr as argument.
            * @return true on success. Otherwise return false.
            */
            bool setIdleTask( taskFcn_t callback ) noexcept;
            bool schedulerRelease( void ) noexcept;
            bool setSchedulerReleaseCallback( taskFcn_t callback ) noexcept;
            /**
            * @brief Remove the task from the scheduling scheme.
            * @param[in] Task The task node.
            * @return Returns true if success, otherwise returns false.
            */
            bool removeTask( task &Task ) noexcept;
            /**
            * @brief Executes the scheduling scheme. It must be called once after the
            * task pool has been defined.
            * @note This call keeps the application in an endless loop.
            * @return true if a release action its performed. In a normal scenario,
            * this function never returns.
            */
            bool run( void ) noexcept;
            /**
            * @brief Sends a notification generating an asynchronous event.
            * If mode = notifyMode::SIMPLE, the method marks the task as 
            * ready for execution and the scheduler planner will launch the task
            * immediately according to the scheduling rules (even
            * if task is disabled) and setting the event_t::Trigger flag to
            * ::byNotificationSimple. 
            * If mode = notifyMode::QUEUED, the notification will insert the 
            * notification in the FIFO priority queue. The scheduler get this 
            * notification as an asynchronous event and the task will be ready 
            * for execution according to the queue order (determined by priority),
            * even if task is in a disabled or sleep operational state. When extracted,
            * the scheduler will set event_t::Trigger flag to  ::byNotificationQueued.
            * Specific user-data can be passed through, and will be available inside the
            * event_t::EventData field, only in corresponding launch. If the task is in
            * a @c qSleep operation state, the scheduler will change the operational state
            * to qAwaken setting the @c SHUTDOWN bit.
            * Specific user-data can be passed through, and will
            * be available in the respective callback inside the qEvent_t::EventData
            * field.
            * @param[in] mode the method used to send the event: notifyMode::SIMPLE
            * or notifyMode::QUEUED.
            * @param[in] Task The task node.
            * @param[in] eventData Specific event user-data.
            * @return true on success. Otherwise false.
            */
            bool notify( notifyMode mode, task &Task, void* eventData = nullptr ) noexcept;
            /**
            * @brief Try to spread a notification among all the tasks in the scheduling
            * scheme
            * @note Operation will be performed in the next scheduling cycle.
            * @param[in] mode the method used to spread the event: notifyMode::SIMPLE
            * or notifyMode::QUEUED.
            * @param[in] eventData Specific event user-data.
            * @return true if success. false if any other spread operation is in
            * progress.
            */
            bool notify( notifyMode mode, void* eventData = nullptr ) noexcept;
            bool hasPendingNotifications( task &Task ) noexcept;
            bool eventFlagsModify( task &Task, const taskFlag_t tFlags, const bool action ) noexcept;
            taskFlag_t eventFlagsRead( task &Task ) const noexcept;
            bool eventFlagsCheck( task &Task, taskFlag_t flagsToCheck, const bool clearOnExit = true, const bool checkForAll = false ) noexcept;
            /**
            * @brief Tries to find the first task that matches the name provided.
            * @param[in] name The string with the name to find.
            * @return A pointer to the task node if found, otherwise returns @c nullptr.
            */
            task* findTaskByName( const char *name ) noexcept;
            /**
            * @brief Yield the control of the current running task to another task.
            * @note This API can only be invoked from the context of a task.
            * @note Target task will inherit the event data.
            * @warning Yielding from the IDLE task is not allowed.
            * @param[in] Task The the task to which current control will
            * be yielded.
            * @return Returns true if success, otherwise returns false.
            */
            bool yieldToTask( task &Task ) noexcept;
            /**
            * @brief Retrieve the task global-state.
            * @param[in] Task The task node.
            * @return One of the available global states : ::WAITING, ::SUSPENDED,
            * ::RUNNING, ::READY.
            * Return ::UNDEFINED if the current task its passing through a
            * current kernel transaction
            */
            globalState getGlobalState( task &Task ) const noexcept;
    };

    extern core& os; /* skipcq: CXX-W2011 */

}

#endif /*QOS_CPP_KERNEL*/
