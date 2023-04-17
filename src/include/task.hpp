#ifndef QOS_CPP_TASK
#define QOS_CPP_TASK

#include "include/types.hpp"
#include "include/timer.hpp"
#include "include/list.hpp"
#include "include/queue.hpp"

namespace qOS {

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

    enum class globalState : uint8_t {
        UNDEFINED,  /**< A task should never reach this state(Reserved for internal use) */
        READY,      /**< The task has completed preparations for running, but cannot run because a task with a higher precedence is running. */
        WAITING,    /**< The task cannot run because the conditions for running are not in place. */
        SUSPENDED,  /**< The task doesn't take part in what is going on. Normally this state is taken after the ::qRunning state or when the task doesn't reach the ::qReady state*/
        RUNNING     /**< The task is currently being executed. */
    };

    class task;

    /**
    * @brief The task argument with all the regarding information of the task
    * execution.
    * @note Should be used only in task-callbacks as the only input argument.
    */
    class _Event {
        protected:
            trigger Trigger{ trigger::None };
            bool FirstCall{ false };
            bool FirstIteration{ false };
            bool LastIteration{ false };
            clock_t StartDelay{ 0u };
            task* currentTask{ nullptr };
            _Event() = default;
        public:
            /**
            * @brief Task arguments defined at the time of its creation.
            * (Storage-Pointer)
            */
            void *TaskData{ nullptr };
            /**
            * @brief Associated data of the event. Specific data will reside here
            * according to the event source. This field will have a @c nullptr value when
            * the trigger gets one of this values: ::byTimeElapsed, ::byEventFlags
            * and ::byNoReadyTasks.
            */
            void *EventData{ nullptr };
            /**
            * @brief Checks whether the task is running for the first time.
            * Can be used for data initialization purposes.
            */
            bool firstCall( void ) const noexcept
            {
                return FirstCall;
            }
            /**
            * @brief Checks whether the current pass is the first iteration of the
            * task.  The value returned by this method will be only true when 
            * time-elapsed events occurs and the Iteration counter has been 
            * parameterized. Asynchronous events never change the task iteration 
            * counter, consequently doesn't have effect in this flag
            */
            bool firstIteration( void ) const noexcept
            {
                return FirstIteration;
            }
            /**
            * @brief Checks whether the current pass is the last iteration of the
            * task. The value returned by this method will be only true when 
            * time-elapsed events occurs and the Iteration counter has been 
            * parameterized. Asynchronous events never change the task iteration
            *  counter, consequently doesn't have effect in the value returned by
            * this method.
            */
            bool lastIteration( void ) const noexcept
            {
                return LastIteration;
            }
            /**
            * @brief Get the event source that triggers the task execution.
            * Possible values are described in the qOS::trigger enum.
            */
            trigger getTrigger( void ) const noexcept
            {
                return Trigger;
            }
            /**
            * @brief return the number of epochs between current system time and
            * point in time when the task was marked as Ready.
            * Can be used to keep track when current task's execution took place
            * relative to when it was scheduled
            * A value of 0 (zero) indicates that task started right on time per
            * schedule.This parameter will be only available on timed tasks. when
            * @c qOS::trigger == ::byTimeElapsed
            */
            clock_t startDelay( void ) const noexcept
            {
                return StartDelay;
            }
            /**
            * @brief return the current task node being evaluated
            */
            task& self( void ) noexcept
            {
                return *currentTask;
            }
    };
    using event_t = _Event&;

    enum class taskState {
        DISABLED_STATE = 0,
        ENABLED_STATE = 1,
        AWAKE_STATE = 2,
        ASLEEP_STATE = 3,
    };

    /**
    * @brief An enum that defines the modes in which a queue can be linked
    * to a task
    */
    enum class queueLinkMode : uint32_t {
        QUEUE_RECEIVER = 4u,    /**< This mode will trigger the task if there are elements in the queue. Data will be extracted automatically in every trigger and will be available in the qEvent_t::EventData field.*/
        QUEUE_FULL = 8u,        /**< This mode will trigger the task if the queue is full. A pointer to the queue will be available in the qEvent_t::EventData field.*/
        QUEUE_COUNT = 16u,      /**< This mode will trigger the task if the count of elements in the queue reach the specified value. A pointer to the queue will be available in the qEvent_t::EventData field.*/
        QUEUE_EMPTY = 32u,      /**< This mode will trigger the task if the queue is empty. A pointer to the queue will be available in the qEvent_t::EventData field.*/
    };

    using taskFcn_t = void (*)( event_t );
    /** @brief A 32-bit unsigned integer type to hold a notification value.*/
    using notifier_t = uint32_t;
    /** @brief A 32-bit unsigned integer type to hold the task flags.*/
    using taskFlag_t = uint32_t;

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
    * Every task node, must be defined using the qTask_t data-type and the
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
    class task : protected node {
        friend class core;
        private:
            void *taskData{ nullptr };
            void *asyncData{ nullptr };
            taskFcn_t callback{ nullptr };
            void *aObj{ nullptr };
            queue *aQueue{ nullptr };
            size_t aQueueCount{ 0uL };
            const char *name{ nullptr };
            timer time;
            cycles_t cycles{ 0uL };
            size_t entry{ static_cast<size_t>( 0xFFFFFFFFu ) };
            iteration_t iterations{ 0 };
            volatile notifier_t notifications{ 0uL };
            volatile taskFlag_t flags{ 0uL };
            priority_t priority{ 0u };
            trigger Trigger{ trigger::None };
            void setFlags( const uint32_t xFlags, const bool value ) noexcept;
            bool getFlag( const uint32_t flag ) const noexcept;
            bool deadLineReached( void ) const noexcept;
            trigger queueCheckEvents( void ) noexcept;
            _Event *pEventInfo{ nullptr };
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
            task( task const& ) = delete;
            void operator=( task const& ) = delete;
        public:
            task() = default;
            priority_t getPriority( void ) const noexcept;
            /**
            * @brief Set/Change the task priority value
            * @param[in] pValue Priority Value. [0(min) - @c Q_PRIORITY_LEVELS (max)]
            * @return true on success. Otherwise return false.
            */
            bool setPriority( priority_t pValue ) noexcept;
            /**
            * @brief Retrieve the number of task activations.
            * @return An unsigned long value containing the number of task
            * activations.
            */
            cycles_t getCycles( void ) const noexcept;
            /**
            * @brief Retrieve the task operational state.
            * @return taskState::ENABLED or taskState::DISABLED if the task is 
            * taskState::AWAKEN. taskState::ASLEEP if the task is in a Sleep 
            * operational state.
            */
            taskState getState( void ) const noexcept;
            /**
            * @brief Set the task operational state
            * @param[in] s Use one of the following values:
            *
            * taskState::ENABLED : Task will be able to catch all the events. 
            * ( @c ENABLE_Bit=1 )
            *
            * taskState::DISABLED : Time events will be discarded. The task 
            * catch asynchronous events. ( @c ENABLE_Bit=0)
            *
            * taskState::ASLEEP : Put the task into a sleep operability state. 
            * The task can't be triggered by the lower precedence events. 
            * ( @c SHUTDOWN_Bit=0)
            *
            * taskState::AWAKE : Put the task into the previous state before it 
            * was put in the sleep state.( @c SHUTDOWN_Bit=1 )
            * @return true on success. Otherwise return false.
            */
            bool setState( taskState s ) noexcept;
            /**
            * @brief Set/Change the number of task iterations
            * @param[in] iValue Number of task executions (Integer value). For
            * indefinite execution (@a iValue = os.PERIODIC or os.INDEFINITE).
            * Tasks do not remember the number of iteration set initially. After
            * the iterations are done, internal iteration counter is 0. If you 
            * need to perform another set of iterations, you need to set the 
            * number of iterations again and resume.
            */
            void setIterations( iteration_t iValue ) noexcept;
            /**
            * @brief Set/Change the Task execution interval
            * @param[in] tValue Execution interval. For immediate execution 
            * (@a tValue = clock::IMMEDIATE).
            * @return true on success, otherwise returns false.
            */
            bool setTime( const qOS::time_t tValue ) noexcept;
            /**
            * @brief Set/Change the task callback function.
            * @note This function can be used to detach a state-machine from a task
            * @param[in] tCallback A pointer to a void callback method with a 
            * event_t parameter as input argument.
            * @return true on success. Otherwise return false.
            */
            bool setCallback( const taskFcn_t tCallback ) noexcept;
            /**
            * @brief Set the task data
            * @param[in] arg The task storage pointer. User data.
            * @return true on success. Otherwise return false.
            */
            bool setData( void *arg ) noexcept;
            /**
            * @brief Set the task name
            * @note Name should be unique.
            * @param[in] name A raw-string with the task name
            * @return true on success. Otherwise return false.
            */
            bool setName( const char *tName ) noexcept;
            /**
            * @brief Retrieves the task name
            * @return A pointer to the string containing the task name.
            */
            const char* getName( void ) const noexcept;
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
            * @return Returns true on success, otherwise returns false.
            */
            bool attachQueue( queue &q, const queueLinkMode mode, const size_t arg = 1u ) noexcept;
            void * const & getAttachedObject( void ) const noexcept;
            event_t eventData( void ) const noexcept;
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

}

#endif /*QOS_CPP_TASK*/
