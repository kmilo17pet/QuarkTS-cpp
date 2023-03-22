#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include "config.h"
#include "list.hpp"
#include "clock.hpp"
#include "types.hpp"
#include "task.hpp"
#include "prioqueue.hpp"
#include "fsm.hpp"

namespace qOS {

    typedef uint32_t coreFlags_t;
    typedef bool (*notificationSpreaderFcn_t)( task * const Task, void* eventData );

    typedef struct _qNotificationSpreader_s {
        notificationSpreaderFcn_t mode;
        void *eventData;
    } notificationSpreader_t;

    typedef enum {
        SIMPLE,
        QUEUED
    } notifyMode;

    class core : protected _Event {
        protected:
            taskFcn_t idleCallback{ nullptr };
            taskFcn_t releaseSchedCallback{ nullptr };
            task *currentTask{ nullptr };
            task *yieldTask{ nullptr };
            queueStack_t pq_stack[ Q_PRIO_QUEUE_SIZE ];
            prioQueue priorityQueue{ pq_stack, sizeof(pq_stack)/sizeof(queueStack_t) };
            //_event_t_ eventInfo;
            volatile coreFlags_t flag{ 0uL };
            notificationSpreader_t nSpreader{ nullptr, nullptr };
            size_t taskEntries{ 0uL };
            list coreLists[ Q_PRIORITY_LEVELS + 2 ];
            list* waitingList = &coreLists[ Q_PRIORITY_LEVELS ];
            list* suspendedList = &coreLists[ Q_PRIORITY_LEVELS + 1 ];
            list* readyList = &coreLists[ 0 ];
            const priority_t MAX_PRIORITY_VALUE = static_cast<priority_t>( Q_PRIORITY_LEVELS ) - 1u;
            const uint32_t BIT_INIT = 0x00000001uL;
            const uint32_t BIT_FCALL_IDLE = 0x00000002uL;
            const uint32_t BIT_RELEASE_SCHED = 0x00000004uL;
            const uint32_t BIT_FCALL_RELEASED = 0x00000008uL;
            void triggerReleaseSchedEvent( void );
            bool checkIfReady( void );
            void dispatchTaskFillEventInfo( task *Task );
            void dispatch( list *xList );
            void dispatchIdle( void );
            core() = default;
        public:
            static const priority_t LOWEST_PRIORITY;
            static const priority_t MEDIUM_PRIORITY;
            static const priority_t HIGHEST_PRIORITY;
            static core& getInstance( void );
            core( core &other ) = delete;
            void operator=( const core & ) = delete;
            
            void init( const getTickFcn_t tFcn, const timingBase_t t, taskFcn_t idleCallback );
            bool addTask( task *Task, taskFcn_t callback, const priority_t p, const time_t t, const iteration_t n, const taskState init, void *arg );
            bool addEventTask( task *Task, taskFcn_t callback, const priority_t p, void *arg );
            bool addStateMachineTask( task *Task, stateMachine *m, const priority_t p, const time_t t, const taskState init, void *arg );
            task* getTaskRunning( void ) const;
            bool setIdleTask( taskFcn_t callback );
            bool schedulerRelease( void );
            bool setSchedulerReleaseCallback( taskFcn_t callback );
            bool removeTask( task *Task );
            bool run( void );
            bool notify( notifyMode mode, task* Task, void* eventData );
            bool hasPendingNotifications( task* Task );
            bool eventFlagsModify( task* Task, const taskFlag_t flags, const bool action );
            taskFlag_t eventFlagsRead( task *Task ) const;
            bool eventFlagsCheck( task *Task, taskFlag_t flagsToCheck, const bool clearOnExit, const bool checkForAll );
            task* findTaskByName( const char *name );
            bool yieldToTask( task *Task );
            globalState getGlobalState( task *Task ) const;
    };

    extern core& os;

}