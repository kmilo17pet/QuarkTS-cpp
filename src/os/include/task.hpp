#ifndef QOS_CPP_TASK
#define QOS_CPP_TASK

#include "types.hpp"
#include "timer.hpp"
#include "list.hpp"
#include "queue.hpp"

namespace qOS {

    enum class trigger : std::uint8_t {
        None = 0,
        byTimeElapsed,
        byNotificationQueued,
        byNotificationSimple,
        byQueueReceiver,
        byQueueFull,
        byQueueCount,
        byQueueEmpty,
        byEventFlags,
        bySchedulingRelease,
        byNoReadyTasks
    } ;

    enum class globalState : std::uint8_t {
        UNDEFINED,             /**< A task should never reach this state(Reserved for internal use) */
        READY,                 /**< The task has completed preparations for running, but cannot run because a task with a higher precedence is running. */
        WAITING,               /**< The task cannot run because the conditions for running are not in place. */
        SUSPENDED,             /**< The task doesn't take part in what is going on. Normally this state is taken after the ::qRunning state or when the task doesn't reach the ::qReady state*/
        RUNNING                /**< The task is currently being executed. */
    };

    class _Event {
        protected:
            trigger Trigger{ trigger::None };
            bool FirstCall{ false };
            bool FirstIteration{ false };
            bool LastIteration{ false };
            clock_t StartDelay{ 0u };
            _Event() = default;
        public:
            void *TaskData{ nullptr };
            void *EventData{ nullptr };
            bool firstCall( void ) const noexcept
            {
                return FirstCall;
            }
            bool firstIteration( void ) const noexcept
            {
                return FirstIteration;
            }
            bool lastIteration( void ) const noexcept
            {
                return LastIteration;
            }
            trigger getTrigger( void ) const noexcept
            {
                return Trigger;
            }
            clock_t startDelay( void ) const noexcept
            {
                return StartDelay;
            }
    };
    using event_t = _Event&;

    enum taskState {
        DISABLED = 0,
        ENABLED = 1,
        AWAKE = 2,
        ASLEEP = 3,
    };

    enum queueLinkMode : std::uint32_t {
        QUEUE_RECEIVER = 4u,    /**< This mode will trigger the task if there are elements in the queue. Data will be extracted automatically in every trigger and will be available in the qEvent_t::EventData field.*/
        QUEUE_FULL = 8u,        /**< This mode will trigger the task if the queue is full. A pointer to the queue will be available in the qEvent_t::EventData field.*/
        QUEUE_COUNT = 16u,      /**< This mode will trigger the task if the count of elements in the queue reach the specified value. A pointer to the queue will be available in the qEvent_t::EventData field.*/
        QUEUE_EMPTY = 32u,      /**< This mode will trigger the task if the queue is empty. A pointer to the queue will be available in the qEvent_t::EventData field.*/
    };

    using taskFcn_t = void (*)( event_t );
    using notifier_t = std::uint32_t;
    using taskFlag_t = std::uint32_t;

    class task : protected node {
        friend class core;
        private:
            void *taskData{ nullptr };
            void *asyncData{ nullptr };
            taskFcn_t callback{ nullptr };
            void *aObj{ nullptr };
            queue *aQueue{ nullptr };
            std::size_t aQueueCount{ 0uL };
            const char *name{ nullptr };
            timer time;
            cycles_t cycles{ 0uL };
            std::size_t entry{ 0xFFFFFFFFuL };
            iteration_t iterations{ 0 };
            volatile notifier_t notifications{ 0uL };
            volatile taskFlag_t flags{ 0uL };
            priority_t priority{ 0u };
            trigger Trigger{ trigger::None };
            void setFlags( const std::uint32_t flags, const bool value ) noexcept;
            bool getFlag( const std::uint32_t flag ) const noexcept;
            bool deadLineReached( void ) const noexcept;
            trigger queueCheckEvents( void ) noexcept;
            _Event *pEventInfo{ nullptr };
            static const std::uint32_t BIT_INIT;
            static const std::uint32_t BIT_ENABLED;
            static const std::uint32_t BIT_QUEUE_RECEIVER;
            static const std::uint32_t BIT_QUEUE_FULL;
            static const std::uint32_t BIT_QUEUE_COUNT;
            static const std::uint32_t BIT_QUEUE_EMPTY;
            static const std::uint32_t BIT_SHUTDOWN;
            static const std::uint32_t BIT_REMOVE_REQUEST;
            static const std::uint32_t EVENT_FLAGS_MASK;
            static const std::uint32_t QUEUE_FLAGS_MASK;
            task( task const& ) = delete;
            void operator=( task const& ) = delete;
        public:
            task() = default;
            priority_t getPriority( void ) const noexcept;
            bool setPriority( priority_t pValue ) noexcept;
            cycles_t getCycles( void ) const noexcept;
            taskState getState( void ) const noexcept;
            bool setState( taskState s ) noexcept;
            void setIterations( iteration_t iValue ) noexcept;
            bool setTime( const qOS::time_t tValue ) noexcept;
            bool setCallback( const taskFcn_t tCallback ) noexcept;
            bool setData( void *arg ) noexcept;
            bool setName( const char *tName ) noexcept;
            const char* getName( void ) const noexcept;
            std::size_t getID( void ) const noexcept;
            bool attachQueue( queue &q, const queueLinkMode mode, const std::size_t arg ) noexcept;
            void * const & getAttachedObject( void ) const noexcept;
            event_t eventData( void ) const noexcept;
            static const iteration_t PERIODIC;
            static const iteration_t INDEFINITE;
            static const iteration_t SINGLE_SHOT;
    };

}

#endif /*QOS_CPP_TASK*/
