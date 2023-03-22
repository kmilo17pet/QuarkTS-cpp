#ifndef QTASK_H
#define QTASK_H

#include <iostream>
#include <cstdint>

#include "types.hpp"
#include "timer.hpp"
#include "list.hpp"
#include "queue.hpp"


namespace qOS {
    typedef enum {
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
    } trigger;

    typedef enum {
        UNDEFINED,  /**< A task should never reach this state(Reserved for internal use) */
        READY,                 /**< The task has completed preparations for running, but cannot run because a task with a higher precedence is running. */
        WAITING,               /**< The task cannot run because the conditions for running are not in place. */
        SUSPENDED,             /**< The task doesn't take part in what is going on. Normally this state is taken after the ::qRunning state or when the task doesn't reach the ::qReady state*/
        RUNNING                /**< The task is currently being executed. */
    } globalState;

    class _Event {
        protected:
            trigger Trigger;
            bool FirstCall;
            bool FirstIteration;
            bool LastIteration;
            clock_t StartDelay;
            _Event() = default;
        public:
            void *TaskData;
            void *EventData;
            bool firstCall( void ) const { return FirstCall; }
            bool firstIteration( void ) const { return FirstIteration; }
            bool lastIteration( void ) const { return LastIteration; }
            trigger getTrigger( void ) const { return Trigger; }
            clock_t startDelay( void ) const { return StartDelay; }
    };
    typedef _Event& event_t;

    typedef enum {
        DISABLED = 0,
        ENABLED = 1,
        AWAKE = 2,
        ASLEEP = 3,
    } taskState;

    typedef enum {
        QUEUE_RECEIVER = 4,    /**< This mode will trigger the task if there are elements in the queue. Data will be extracted automatically in every trigger and will be available in the qEvent_t::EventData field.*/
        QUEUE_FULL = 8,        /**< This mode will trigger the task if the queue is full. A pointer to the queue will be available in the qEvent_t::EventData field.*/
        QUEUE_COUNT = 16,      /**< This mode will trigger the task if the count of elements in the queue reach the specified value. A pointer to the queue will be available in the qEvent_t::EventData field.*/
        QUEUE_EMPTY = 32,      /**< This mode will trigger the task if the queue is empty. A pointer to the queue will be available in the qEvent_t::EventData field.*/
    } queueLinkMode;

    typedef void (*taskFcn_t)( event_t arg );
    typedef uint32_t notifier_t;
    typedef uint32_t taskFlag_t;

    class task : protected node {
        friend class core;
        protected:
            void *taskData{ nullptr };
            void *asyncData{ nullptr };
            taskFcn_t callback{ nullptr };
            void *aObj{ nullptr };
            queue *aQueue{ nullptr };
            size_t aQueueCount{ 0uL };
            const char *name{ nullptr };
            timer time;
            cycles_t cycles{ 0uL };
            size_t entry{ 0xFFFFFFFFuL };
            iteration_t iterations;
            volatile notifier_t notifications{ 0uL };
            volatile taskFlag_t flags{ 0uL };
            priority_t priority;
            trigger Trigger{ trigger::None };
            void setFlags( const uint32_t flags, const bool value );
            bool getFlag( const uint32_t flag ) const;
            bool deadLineReached( void ) const;
            trigger queueCheckEvents( void );
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

            task( task const& ) = delete;      /* not copyable*/
            void operator=( task const& ) = delete;  /* not assignable*/
        public:
            task() = default;
            priority_t getPriority( void ) const;
            bool setPriority( priority_t pValue );
            cycles_t getCycles( void ) const;
            taskState getState( void ) const;
            bool setState( taskState s );
            void setIterations( iteration_t iValue );
            bool setTime( const time_t tValue );
            bool setCallback( taskFcn_t callback );
            bool setData( void *arg );
            bool setName( const char *name );
            const char* getName( void ) const;
            size_t getID( void ) const;
            bool attachQueue( queue &q, const queueLinkMode mode, const size_t arg );
            void* getAttachedObject( void ) const;
            event_t eventData( void ) const;

            static iteration_t PERIODIC;
            static iteration_t INDEFINITE;
            static iteration_t SINGLE_SHOT;
    };

}

#endif