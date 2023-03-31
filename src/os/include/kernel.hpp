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
#if ( Q_CLI == 1 )
    #include "cli.hpp"
#endif

namespace qOS {

    using coreFlags_t = std::uint32_t ;

    enum class notifyMode : std::uint8_t {
        SIMPLE = 0u,
        QUEUED = 1u,
        _NONE_ = 3u,
    };
    struct _notificationSpreader_s {
        notifyMode mode;
        void *eventData;
    };
    using notificationSpreader_t = struct _notificationSpreader_s;

    class core : protected _Event {
        private:
            taskFcn_t idleCallback{ nullptr };
            taskFcn_t releaseSchedCallback{ nullptr };
            task *currentTask{ nullptr };
            task *yieldTask{ nullptr };
            queueStack_t pq_stack[ Q_PRIO_QUEUE_SIZE ];
            prioQueue priorityQueue{ pq_stack, sizeof(pq_stack)/sizeof(queueStack_t) };
            volatile coreFlags_t flag{ 0uL };
            notificationSpreader_t nSpreader{ notifyMode::_NONE_, nullptr };
            std::size_t taskEntries{ 0uL };
            list coreLists[ Q_PRIORITY_LEVELS + 2 ];
            list& waitingList{ coreLists[ Q_PRIORITY_LEVELS ] };
            list& suspendedList{ coreLists[ Q_PRIORITY_LEVELS + 1 ] };
            list& readyList{ coreLists[ 0 ] };
            const priority_t MAX_PRIORITY_VALUE = static_cast<priority_t>( Q_PRIORITY_LEVELS ) - 1u;
            const std::uint32_t BIT_INIT = 0x00000001uL;
            const std::uint32_t BIT_FCALL_IDLE = 0x00000002uL;
            const std::uint32_t BIT_RELEASE_SCHED = 0x00000004uL;
            const std::uint32_t BIT_FCALL_RELEASED = 0x00000008uL;
            void triggerReleaseSchedEvent( void ) noexcept;
            bool checkIfReady( void ) noexcept;
            void dispatchTaskFillEventInfo( task *Task ) noexcept;
            void dispatch( list * const xList ) noexcept;
            void dispatchIdle( void ) noexcept;
            core() = default;
        public:
            static const priority_t LOWEST_PRIORITY;
            static const priority_t MEDIUM_PRIORITY;
            static const priority_t HIGHEST_PRIORITY;
            static core& getInstance( void ) noexcept;
            core( core &other ) = delete;
            void operator=( const core & ) = delete;
            
            void init( const getTickFcn_t tFcn, const timingBase_t t, taskFcn_t idleCallback ) noexcept;
            bool addTask( task &Task, taskFcn_t callback, const priority_t p, const time_t t, const iteration_t n, const taskState s, void *arg ) noexcept;
            inline bool addTask( task &Task, taskFcn_t callback, const priority_t p, const time_t t, const iteration_t n, const taskState s ) noexcept
            {
                return addTask( Task, callback, p, t, n, s, nullptr );
            }
            inline bool addTask( task &Task, taskFcn_t callback, const priority_t p, const time_t t, const iteration_t n ) noexcept
            {
                return addTask( Task, callback, p, t, n, taskState::ENABLED, nullptr );
            }
            inline bool addEventTask( task &Task, taskFcn_t callback, const priority_t p, void *arg ) noexcept
            {
                return addTask( Task, callback, p, clock::IMMEDIATE, task::SINGLE_SHOT, taskState::DISABLED, arg );
            }
            inline bool addEventTask( task &Task, taskFcn_t callback, const priority_t p ) noexcept
            {
                return addTask( Task, callback, p, clock::IMMEDIATE, task::SINGLE_SHOT, taskState::DISABLED, nullptr );
            }
            #if ( Q_FSM == 1 )
            bool addStateMachineTask( task &Task, stateMachine &m, const priority_t p, const time_t t, const taskState s, void *arg ) noexcept;
            inline bool addStateMachineTask( task &Task, stateMachine &m, const priority_t p, const time_t t, const taskState s ) noexcept
            {
                return addStateMachineTask( Task, m, p, t, s, nullptr );
            }
            inline bool addStateMachineTask( task &Task, stateMachine &m, const priority_t p, const time_t t ) noexcept
            {
                return addStateMachineTask( Task, m, p, t, taskState::ENABLED, nullptr );
            }
            #endif
            #if ( Q_CLI == 1 )
            bool addCommandLineInterfaceTask( task &Task, commandLineInterface &cli, const priority_t p, void *arg ) noexcept;
            inline bool addCommandLineInterfaceTask( task &Task, commandLineInterface &cli, const priority_t p ) noexcept
            {
                return addCommandLineInterfaceTask( Task, cli, p );
            }
            #endif
            task& getTaskRunning( void ) const noexcept;
            bool setIdleTask( taskFcn_t callback ) noexcept;
            bool schedulerRelease( void ) noexcept;
            bool setSchedulerReleaseCallback( taskFcn_t callback ) noexcept;
            bool removeTask( task &Task ) noexcept;
            bool run( void ) noexcept;
            bool notify( notifyMode mode, task &Task, void* eventData ) noexcept;
            inline bool notify( notifyMode mode, task &Task ) noexcept
            {
                return notify( mode, Task, nullptr );
            }
            bool notify( notifyMode mode, void* eventData ) noexcept;
            inline bool notify( notifyMode mode ) noexcept
            {
                return notify( mode, nullptr );
            }
            bool hasPendingNotifications( task &Task ) noexcept;
            bool eventFlagsModify( task &Task, const taskFlag_t tFlags, const bool action ) noexcept;
            taskFlag_t eventFlagsRead( task &Task ) const noexcept;
            bool eventFlagsCheck( task &Task, taskFlag_t flagsToCheck, const bool clearOnExit, const bool checkForAll ) noexcept;
            task* findTaskByName( const char *name ) noexcept;
            bool yieldToTask( task &Task ) noexcept;
            globalState getGlobalState( task &Task ) const noexcept;
    };

    extern core& os; /* skipcq: CXX-W2011 */

}

#endif /*QOS_CPP_KERNEL*/
