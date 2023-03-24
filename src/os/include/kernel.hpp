#ifndef QOS_CPP_KERNEL
#define QOS_CPP_KERNEL

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

#if ( Q_FSM == 1 )
    #include "fsm.hpp"
#endif
#if ( Q_ATCLI == 1 )
    #include "cli.hpp"
#endif

namespace qOS {

    using coreFlags_t = std::uint32_t ;
    using notificationSpreaderFcn_t = bool (*)( task * const, void* );

    struct _notificationSpreader_s {
        notificationSpreaderFcn_t mode;
        void *eventData;
    };
    using notificationSpreader_t = struct _notificationSpreader_s;

    enum class notifyMode : std::uint8_t {
        SIMPLE,
        QUEUED
    };

    class core : protected _Event {
        protected:
            taskFcn_t idleCallback{ nullptr };
            taskFcn_t releaseSchedCallback{ nullptr };
            task *currentTask{ nullptr };
            task *yieldTask{ nullptr };
            queueStack_t pq_stack[ Q_PRIO_QUEUE_SIZE ];
            prioQueue priorityQueue{ pq_stack, sizeof(pq_stack)/sizeof(queueStack_t) };
            volatile coreFlags_t flag{ 0uL };
            notificationSpreader_t nSpreader{ nullptr, nullptr };
            std::size_t taskEntries{ 0uL };
            list coreLists[ Q_PRIORITY_LEVELS + 2 ];
            list* waitingList = &coreLists[ Q_PRIORITY_LEVELS ];
            list* suspendedList = &coreLists[ Q_PRIORITY_LEVELS + 1 ];
            list* readyList = &coreLists[ 0 ];
            const priority_t MAX_PRIORITY_VALUE = static_cast<priority_t>( Q_PRIORITY_LEVELS ) - 1u;
            const std::uint32_t BIT_INIT = 0x00000001uL;
            const std::uint32_t BIT_FCALL_IDLE = 0x00000002uL;
            const std::uint32_t BIT_RELEASE_SCHED = 0x00000004uL;
            const std::uint32_t BIT_FCALL_RELEASED = 0x00000008uL;
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
            bool addTask( task &Task, taskFcn_t callback, const priority_t p, const time_t t, const iteration_t n, const taskState init, void *arg );
            bool addTask( task &Task, taskFcn_t callback, const priority_t p, const time_t t, const iteration_t n, const taskState init )
            {
                return addTask( Task, callback, p, t, n, init, nullptr );
            }
            bool addTask( task &Task, taskFcn_t callback, const priority_t p, const time_t t, const iteration_t n )
            {
                return addTask( Task, callback, p, t, n, taskState::ENABLED, nullptr );
            }
            bool addEventTask( task &Task, taskFcn_t callback, const priority_t p, void *arg )
            {
                return addTask( Task, callback, p, clock::IMMEDIATE, task::SINGLE_SHOT, taskState::DISABLED, arg );
            }
            bool addEventTask( task &Task, taskFcn_t callback, const priority_t p )
            {
                return addTask( Task, callback, p, clock::IMMEDIATE, task::SINGLE_SHOT, taskState::DISABLED, nullptr );
            }
            #if ( Q_FSM == 1 )
            bool addStateMachineTask( task &Task, stateMachine &m, const priority_t p, const time_t t, const taskState init, void *arg );
            bool addStateMachineTask( task &Task, stateMachine &m, const priority_t p, const time_t t, const taskState init )
            {
                return addStateMachineTask( Task, m, p, t, init, nullptr );
            }
            bool addStateMachineTask( task &Task, stateMachine &m, const priority_t p, const time_t t )
            {
                return addStateMachineTask( Task, m, p, t, taskState::ENABLED, nullptr );
            }
            #endif
            #if ( Q_ATCLI == 1 )
            bool addCommandLineInterfaceTask( task &Task, commandLineInterface &cli, const priority_t p, void *arg );
            bool addCommandLineInterfaceTask( task &Task, commandLineInterface &cli, const priority_t p )
            {
                return addCommandLineInterfaceTask( Task, cli, p );
            }
            #endif
            task& getTaskRunning( void ) const;
            bool setIdleTask( taskFcn_t callback );
            bool schedulerRelease( void );
            bool setSchedulerReleaseCallback( taskFcn_t callback );
            bool removeTask( task &Task );
            bool run( void );
            bool notify( notifyMode mode, task &Task, void* eventData );
            bool hasPendingNotifications( task &Task );
            bool eventFlagsModify( task &Task, const taskFlag_t flags, const bool action );
            taskFlag_t eventFlagsRead( task &Task ) const;
            bool eventFlagsCheck( task &Task, taskFlag_t flagsToCheck, const bool clearOnExit, const bool checkForAll );
            task* findTaskByName( const char *name );
            bool yieldToTask( task &Task );
            globalState getGlobalState( task &Task ) const;
    };

    extern core& os;

}

#endif /*QOS_CPP_KERNEL*/