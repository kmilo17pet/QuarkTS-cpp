#ifndef QOS_CPP_TASK
#define QOS_CPP_TASK

#include "include/types.hpp"
#include "include/timer.hpp"
#include "include/list.hpp"
#include "include/queue.hpp"

namespace qOS {

    /** @addtogroup qtaskmanip
    * @brief API interface to manage tasks.
    * @pre In order to be able to manage a task, make sure the task has already
    * been added to the scheduling scheme by using one of the core::addk()
    * overloads
    *  @{
    */

    /**
    * @brief An enum with all the possible values for the event_t::getTrigger()
    * method.
    */
    enum class trigger : uint8_t {
        /**
        * @brief To indicate the absence of trigger. Reserved for internal use.
        */
        None = 0,
        /**
        * @brief When the time specified for the task elapsed.
        */
        byTimeElapsed,
        /**
        * @brief When there is a queued notification in the FIFO priority queue.
        * For this trigger, the dispatcher performs a dequeue operation
        * automatically. A pointer to the extracted event data will be available
        * in the event_t::EventData field.
        */
        byNotificationQueued,
        /**
        * @brief When the execution chain does, according to a requirement of
        * asynchronous notification event prompted by core::notify().
        * A pointer to the notified data will be available in the
        * event_t::EventData field.
        */
        byNotificationSimple,
        /**
        * @brief When there are elements available in the attached queue, the
        * scheduler make a data dequeue (auto-receive) from the front. A pointer
        * to the received data will be available in the event_t::EventData
        * field.
        */
        byQueueReceiver,
        /**
        * @brief When the attached queue is full. A pointer to the queue will be
        * available in the event_t::EventData field.
        */
        byQueueFull,
        /**
        * @brief When the element-count of the attached queue reaches the
        * specified value. A pointer to the queue will be available in the
        * event_t::EventData field.
        */
        byQueueCount,
        /**
        * @brief When the attached queue is empty. A pointer to the queue will
        * be available in the event_t::EventData field.
        */
        byQueueEmpty,
        /**
        * @brief When any event-flag is set.
        */
        byEventFlags,
        /**
        * @brief When the scheduler is released
        */
        bySchedulingRelease,
        /**
        * @brief Only available when the Idle Task is triggered.
        */
        byNoReadyTasks
    };

    /**
    * @brief An enum to describe the task global states.
    */
    enum class globalState : uint8_t {
        UNDEFINED,  /**< A task should never reach this state(Reserved for internal use) */
        READY,      /**< The task has completed preparations for running, but cannot run because a task with a higher precedence is running. */
        WAITING,    /**< The task cannot run because the conditions for running are not in place. */
        SUSPENDED,  /**< The task doesn't take part in what is going on. Normally this state is taken after the globalState::RUNNING state or when the task doesn't reach the globalState::READY state*/
        RUNNING     /**< The task is currently being executed. */
    };

    /*! @cond  */
    class task;
    /*! @endcond  */


    #ifdef DOXYGEN
    /**
    * @brief The task argument with all the regarding information of the task
    * execution.
    * @note Should be used only inside task-callbacks as the only input argument.
    */
    class event_t {
        protected:
            /*! @cond  */
            trigger Trigger{ trigger::None };
            bool FirstCall{ false };
            bool FirstIteration{ false };
            bool LastIteration{ false };
            clock_t StartDelay{ 0u };
            task* currentTask{ nullptr };
            _Event() = default;
            /*! @endcond  */
        public:
            /**
            * @brief Task arguments defined at the time of its creation.
            * (Storage-Pointer)
            */
            void *TaskData{ nullptr };
            /**
            * @brief Associated data of the event. Specific data will reside here
            * according to the event source. This field will have a @c nullptr value when
            * the trigger gets one of this values: trigger::byTimeElapsed,
            * trigger::byEventFlags and trigger::byNoReadyTasks.
            */
            void *EventData{ nullptr };
            /**
            * @brief Checks whether the task is running for the first time.
            * Can be used for data initialization purposes.
            */
            inline bool firstCall( void ) const noexcept
            {
                return FirstCall;
            }
            /**
            * @brief Checks whether the current pass is the first iteration of the
            * task.  The value returned by this method will be only @c true when
            * time-elapsed events occurs and the Iteration counter has been
            * parameterized. Asynchronous events never change the task iteration
            * counter, consequently doesn't have effect in this flag
            */
            inline bool firstIteration( void ) const noexcept
            {
                return FirstIteration;
            }
            /**
            * @brief Checks whether the current pass is the last iteration of the
            * task. The value returned by this method will be only @c true when
            * time-elapsed events occurs and the Iteration counter has been
            * parameterized. Asynchronous events never change the task iteration
            *  counter, consequently doesn't have effect in the value returned by
            * this method.
            */
            inline bool lastIteration( void ) const noexcept
            {
                return LastIteration;
            }
            /**
            * @brief Get the event source that triggers the task execution.
            * Possible values are described in the qOS::trigger enum.
            */
            inline trigger getTrigger( void ) const noexcept
            {
                return Trigger;
            }
            /**
            * @brief return the number of epochs between current system time and
            * point in time when the task was marked as @c READY.
            * Can be used to keep track when current task's execution took place
            * relative to when it was scheduled
            * A value of 0 (zero) indicates that task started right on time per
            * schedule.This parameter will be only available on timed tasks. when
            * @c qOS::trigger == trigger::byTimeElapsed
            */
            inline clock_t startDelay( void ) const noexcept
            {
                return StartDelay;
            }
            /**
            * @brief return the current task node being evaluated
            */
            inline task& thisTask( void ) noexcept
            {
                return *currentTask;
            }
    };
    #endif

    /*! @cond  */
    class taskEvent {
        protected:
            trigger Trigger{ trigger::None };
            bool FirstCall{ false };
            bool FirstIteration{ false };
            bool LastIteration{ false };
            clock_t StartDelay{ 0U };
            task* currentTask{ nullptr };
            taskEvent() = default;
        public:
            /*! @cond  */
            virtual ~taskEvent(){}
            /*! @endcond  */
            void *TaskData{ nullptr };
            void *EventData{ nullptr };
            inline bool firstCall( void ) const noexcept
            {
                return FirstCall;
            }
            inline bool firstIteration( void ) const noexcept
            {
                return FirstIteration;
            }
            inline bool lastIteration( void ) const noexcept
            {
                return LastIteration;
            }
            inline trigger getTrigger( void ) const noexcept
            {
                return Trigger;
            }
            inline clock_t startDelay( void ) const noexcept
            {
                return StartDelay;
            }
            inline task& self( void ) noexcept
            {
                return *currentTask;
            }
            inline task& thisTask( void ) noexcept
            {
                return *currentTask;
            }
    };
    using event_t = taskEvent&;
    /*! @endcond  */


    /**
    * @brief An enum that defines the possible task operational states
    * @details Each task has independent operating states from those globally
    * controlled by the scheduler. These states can be handled by the application
    * writer to modify the event flow to the task and consequently, affect the
    * transition to the globalState::READY global state
    */
    enum class taskState {
        /**
        * @brief In this state, the time events will be discarded. This
        * operational state is available when the @c ENABLE bit is cleared.
        */
        DISABLED_STATE = 0,
        /**
        * @brief The task can catch all the events. This operational state is
        * available when the @c ENABLE bit is set.
        */
        ENABLED_STATE = 1,
        /**
        * @brief In this state, the task is conceptually in an alert mode,
        * handling most of the available events. This operational state is
        * available when the @c SHUTDOWN bit is set, allowing the next operational
        * states to be available:
        */
        AWAKE_STATE = 2,
        /**
        * @brief Task operability is put into a deep doze mode, so the task
        * can not be triggered by the lower precedence events. This operational
        * state is available when the @c SHUTDOWN bit is cleared. The task can exit
        * from this operational state when it receives a high precedence event
        * (a queued notification) or using the task::setState() method.
        */
        ASLEEP_STATE = 3,
    };

    /**
    * @brief An enum that defines the modes in which a queue can be linked
    * to a task
    */
    enum class queueLinkMode : uint32_t {
        QUEUE_RECEIVER = 4U,    /**< This mode will trigger the task if there are elements in the queue. Data will be extracted automatically in every trigger and will be available in the event_t::EventData field.*/
        QUEUE_FULL = 8U,        /**< This mode will trigger the task if the queue is full. A pointer to the queue will be available in the event_t::EventData field.*/
        QUEUE_COUNT = 16U,      /**< This mode will trigger the task if the count of elements in the queue reach the specified value. A pointer to the queue will be available in the event_t::EventData field.*/
        QUEUE_EMPTY = 32U,      /**< This mode will trigger the task if the queue is empty. A pointer to the queue will be available in the event_t::EventData field.*/
    };

    /**
    * @brief Pointer to a task callback.
    */
    using taskFcn_t = void (*)( event_t );
    /** @brief A 32-bit unsigned integer type to hold a notification value.*/
    using notifier_t = uint32_t;

    /** @addtogroup qeventflags
    * @brief API interface for the built-in-task @ref qeventflags.
    *  @{
    */

    /** @brief A 32-bit unsigned integer type to hold the task flags.*/
    using taskFlag_t = uint32_t;

    /** @}*/

    /**
    * @brief A task node object
    * @details Like many operating systems, the basic unit of work is the task.
    * Tasks can perform certain functions, which could require periodic or
    * one-time execution, update of specific variables or waiting for specific
    * events. Tasks also could be controlling specific hardware or be triggered
    * by hardware interrupts. In the QuarkTS OS, a task is seen as a node
    * concept that links together:
    *
    * - Program code performing specific task activities (callback function)
    * - Execution interval (time)
    * - Number of execution (iterations)
    * - Event-based data
    *
    * The OS uses a Task Control Block(TCB) to represent each task, storing
    * essential information about task management and execution. Part of this
    * information also includes link-pointers that allows it to be part of one
    * of the lists available in the Kernel Control Block (KCB).
    *
    * Each task performs its activities via a callback function and each of them
    * is responsible for supporting cooperative multitasking by being
    * “good neighbors”, i.e., running their callback methods quickly in a
    * non-blocking way and releasing control back to the scheduler as soon as
    * possible (returning).
    * Every task node, must be defined using the qOS::task class and the
    * callback is defined as a function that returns void and takes a event_t
    * data structure as its only parameter (This input argument can be used
    * later to get event information.
    *
    * Example :
    * @code{.c}
    * using namespace qOS;
    *
    * task UserTask;
    *
    * void UserTask_Callback( event_t e ) {
    *
    * }
    * @endcode
    *
    * @attention All tasks in QuarkTS must ensure their completion to return the
    * CPU control back to the scheduler, otherwise, the scheduler will hold the
    * execution-state for that task, preventing the activation of other tasks.
    * @note Do not access any member of this structure directly.
    */
    class task : protected node, private nonCopyable {
        friend class core;
        private:
            void *taskData{ nullptr };
            void *asyncData{ nullptr };
            taskFcn_t callback{ nullptr };
            void *aObj{ nullptr };
            queue *aQueue{ nullptr };
            size_t aQueueCount{ 0UL };
            char name[ 11 ] = ""; // skipcq: CXX-W2066, CXX-W2066
            timer time;
            cycles_t cycles{ 0UL };
            size_t entry{ SIZE_MAX };
            iteration_t iterations{ 0 };
            volatile notifier_t notifications{ 0UL };
            volatile taskFlag_t flags{ 0UL };
            priority_t priority{ 0U };
            trigger Trigger{ trigger::None };
            void setFlags( const uint32_t xFlags, const bool value ) noexcept;
            bool getFlag( const uint32_t flag ) const noexcept;
            bool deadLineReached( void ) const noexcept;
            trigger queueCheckEvents( void ) noexcept;
            static const uint32_t BIT_INIT;
            static const uint32_t BIT_ENABLED;
            static const uint32_t BIT_QUEUE_RECEIVER;
            static const uint32_t BIT_QUEUE_FULL;
            static const uint32_t BIT_QUEUE_COUNT;
            static const uint32_t BIT_QUEUE_EMPTY;
            static const uint32_t BIT_SHUTDOWN;
            static const uint32_t BIT_REMOVE_REQUEST;
            static const uint32_t EVENT_FLAGS_MASK;
            static const uint32_t QUEUE_FLAGS_MASK;
        protected:
            virtual void activities( event_t e );
        public:
            task() = default;
            virtual ~task(){}
            /**
            * @brief Retrieve the current task priority.
            * @return The task priority value.
            */
            priority_t getPriority( void ) const noexcept;
            /**
            * @brief Set/Change the task priority value
            * @param[in] pValue Priority Value. [0(min) - @c Q_PRIORITY_LEVELS (max)]
            * @return @c true on success. Otherwise return @c false.
            */
            bool setPriority( const priority_t pValue ) noexcept;
            /**
            * @brief Retrieve the number of task activations.
            * @return An unsigned long value containing the number of task
            * activations.
            */
            cycles_t getCycles( void ) const noexcept;
            /**
            * @brief Retrieve the task operational state.
            * @return taskState::ENABLED_STATE or taskState::DISABLED_STATE if the task is
            * taskState::AWAKE_STATE. taskState::ASLEEP_STATE if the task is in a Sleep
            * operational state.
            */
            taskState getState( void ) const noexcept;
            /**
            * @brief Set the task operational state
            * @param[in] s Use one of the following values:
            *
            * taskState::ENABLED_STATE : Task will be able to catch all the events.
            * ( @c ENABLE_Bit=1 )
            *
            * taskState::DISABLED_STATE : Time events will be discarded. The task
            * catch asynchronous events. ( @c ENABLE_Bit=0)
            *
            * taskState::ASLEEP_STATE : Put the task into a sleep operability state.
            * The task can't be triggered by the lower precedence events.
            * ( @c SHUTDOWN_Bit=0)
            *
            * taskState::AWAKE_STATE : Put the task into the previous state before it
            * was put in the sleep state.( @c SHUTDOWN_Bit=1 )
            * @return @c true on success. Otherwise return @c false.
            */
            bool setState( const taskState s ) noexcept;
            /**
            * @brief Put the task into a disabled state.
            * @see task::setState()
            * @return @c true on success. Otherwise return @c false.
            */
            inline bool suspend( void ) noexcept
            {
                return setState( taskState::DISABLED_STATE );
            }
            /**
            * @brief Put the task into a disabled state.
            * @see task::setState()
            * @return @c true on success. Otherwise return @c false.
            */
            inline bool disable( void ) noexcept
            {
                return setState( taskState::DISABLED_STATE );
            }
            /**
            * @brief Put the task into a enabled state.
            * @see task::setState()
            * @return @c true on success. Otherwise return @c false.
            */
            inline bool resume( void ) noexcept
            {
                return setState( taskState::ENABLED_STATE );
            }
            /**
            * @brief Put the task into a enabled state.
            * @see task::setState()
            * @return @c true on success. Otherwise return @c false.
            */
            inline bool enable( void ) noexcept
            {
                return setState( taskState::ENABLED_STATE );
            }
            /**
            * @brief Put the task into a sleep state. The task can't be triggered
            * by the lower precedence events.
            * @note Only the higher precedence events (Queued Notifications) can
            * wake up the task.
            * @see task::setState()
            * @return @c true on success. Otherwise return @c false.
            */
            inline bool asleep( void ) noexcept
            {
                return setState( taskState::ASLEEP_STATE );
            }
            /**
            * @brief Put the task into a normal operation state. Here the task
            * will be able to catch any kind of events.
            * @see task::setState()
            * @return @c true on success. Otherwise return @c false.
            */
            inline bool awake( void ) noexcept
            {
                return setState( taskState::AWAKE_STATE );
            }
            /**
            * @brief Retrieve the enabled/disabled state
            * @see task::getState()
            * @return @c true if the task in on Enabled state, otherwise returns @c false.
            */
            inline bool isEnabled( void ) const noexcept
            {
                return ( taskState::ENABLED_STATE == getState() );
            }
            /**
            * @brief Set/Change the number of task iterations
            * @param[in] iValue Number of task executions (Integer value). For
            * indefinite execution (@a iValue = task::PERIODIC or task::INDEFINITE).
            * Tasks do not remember the number of iteration set initially. After
            * the iterations are done, internal iteration counter is 0. If you
            * need to perform another set of iterations, you need to set the
            * number of iterations again and resume.
            */
            void setIterations( const iteration_t iValue ) noexcept;
            /**
            * @brief Set/Change the Task execution interval
            * @param[in] tValue Execution interval given in milliseconds. For
            * immediate execution (@a tValue = clock::IMMEDIATE).
            * @return @c true on success, otherwise returns @c false.
            */
            bool setTime( const qOS::duration_t tValue ) noexcept;
            /**
            * @brief Set/Change the task callback function.
            * @note This function can be used to detach a state-machine from a task
            * @param[in] tCallback A pointer to a void callback method with a
            * event_t parameter as input argument.
            * @return @c true on success. Otherwise return @c false.
            */
            bool setCallback( const taskFcn_t tCallback ) noexcept;
            /**
            * @brief Set the task data
            * @param[in] arg The task storage pointer. User data.
            * @return @c true on success. Otherwise return @c false.
            */
            bool setData( void *arg ) noexcept;
            /**
            * @brief Set the task name
            * @pre The task must already be added to the scheduling scheme
            * @note Name should be unique.
            * @note The name @c idle is reserved for the Idle task
            * @remark The size of the string must be less than 11
            * @param[in] tName A raw-string with the task name
            * @return @c true on success. Otherwise return @c false.
            */
            bool setName( const char *tName ) noexcept;
            /**
            * @brief Retrieves the task name
            * @return A pointer to the string containing the task name. If the
            * task is unnamed an empty string will be returned.
            */
            const char* getName( void ) const noexcept;
            /**
            * @brief Retrieves the Task ID number
            * @return The task ID
            */
            size_t getID( void ) const noexcept;
            /**
            * @brief Attach a queue to the Task.
            * @param[in] q The queue object
            * @param[in] mode Attach mode. This implies the event that will trigger
            * the task according to one of the following modes:
            *
            * queueLinkMode::QUEUE_RECEIVER : The task will be triggered if there are
            * elements in the Queue. Data will be extracted automatically in every
            * trigger and will be available in the event_t::EventData field.
            *
            * queueLinkMode::QUEUE_FULL : The task will be triggered if the Queue
            * is full. A pointer to the queue will be available in the
            * event_t::EventData field.
            *
            * queueLinkMode::QUEUE_COUNT : The task will be triggered if the count of
            * elements in the queue reach the specified value. A pointer to the
            * queue will be available in the event_t::EventData field.
            *
            * queueLinkMode::QUEUE_EMPTY : The task will be triggered if the queue
            * is empty. A pointer to the queue will be available in the
            * event_t::EventData field.
            * @param[in] arg This argument defines if the queue will be attached
            * (1u) or detached (0u) from the task.
            * If the queueLinkMode::QUEUE_COUNT mode is specified, this value
            * will be used to check the element count of the queue. A zero value
            * will act as a detach action.
            * @return Returns @c true on success, otherwise returns @c false.
            */
            bool attachQueue( queue &q, const queueLinkMode mode, const size_t arg = 1U ) noexcept;
            /**
            * @brief Retrieves a pointer to the object binded to the task, could
            * be either a state-machine or a command-line-interface
            * @return A @c void pointer to the attached object.
            */
            void * const & getBindedObject( void ) const noexcept;
            /**
            * @brief Retrieves a pointer to the queue attached to the task.
            * @return A @c void pointer to the attached object.
            */
            queue* getQueue( void ) noexcept;
            /**
            * @brief Check if the task has been added to the OS scheduler.
            * @return @c true if task is already added to the scheduler
            */
            explicit operator bool() const noexcept {
                return ( SIZE_MAX != entry ) && ( nullptr == getContainer() );
            }
            /** @brief A constant to indicate that the task will run every time
            * its timeout has expired.
            */
            static const iteration_t PERIODIC;
            /** @brief Same as PERIODIC. A constant to indicate that the task
            * will run every time its timeout has expired.
            */
            static const iteration_t INDEFINITE;
            /** @brief A constant to indicate that the task will be executed
            * only once after its time has expired.
            */
            static const iteration_t SINGLE_SHOT;
    };

    /** @}*/
}


#endif /*QOS_CPP_TASK*/
