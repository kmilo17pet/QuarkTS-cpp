#include "include/kernel.hpp"
#include "include/critical.hpp"
#include "include/helper.hpp"

namespace qOS {
    core& os = core::getInstance(); // skipcq: CXX-W2011
}

using namespace qOS;

static bool taskEntryOrderPreserver( const void *n1, const void *n2 );

const priority_t core::MAX_PRIORITY_VALUE = static_cast<priority_t>( Q_PRIORITY_LEVELS ) - 1u;
const uint32_t core::BIT_INIT = 0x00000001uL;
const uint32_t core::BIT_FCALL_IDLE = 0x00000002uL;
const uint32_t core::BIT_RELEASE_SCHED = 0x00000004uL;
const uint32_t core::BIT_FCALL_RELEASED = 0x00000008uL;

const priority_t core::LOWEST_PRIORITY = 0u;
const priority_t core::MEDIUM_PRIORITY = static_cast<priority_t>( Q_PRIORITY_LEVELS ) >> 1u;
const priority_t core::HIGHEST_PRIORITY = static_cast<priority_t>( Q_PRIORITY_LEVELS ) - 1u;
const notifier_t MAX_NOTIFICATION_VALUE = UINT32_MAX - 1uL;

#if ( Q_CLI == 1 )
    static void fsmTaskCallback( event_t e );
    static void cliTaskCallback( event_t e );
    static void cliNotifyFcn( commandLineInterface *cli );
#endif

/*============================================================================*/
core& core::getInstance( void ) noexcept
{
    static core instance;
    return instance;
}
/*============================================================================*/
/*cstat -MISRAC++2008-7-1-2*/
void core::init( const getTickFcn_t tFcn, taskFcn_t callbackIdle ) noexcept
{
    (void)clock::setTickProvider( tFcn );
    (void)idle.setName( "idle" );
    (void)idle.setPriority( core::LOWEST_PRIORITY );
    (void)idle.setState( taskState::DISABLED_STATE );
    (void)idle.setCallback( callbackIdle );
    idle.entry = 0u;
}
/*cstat +MISRAC++2008-7-1-2*/
/*============================================================================*/
bool core::addTask( task &Task, taskFcn_t callback, const priority_t p, const qOS::time_t t, const iteration_t n, const qOS::taskState s, void *arg ) noexcept
{
    (void)Task.setCallback( callback );
    (void)Task.time.set( t );
    (void)Task.setData( arg );
    (void)Task.setPriority( clipUpper( p, MAX_PRIORITY_VALUE ) );
    (void)Task.setIterations( n );
    Task.setFlags( task::BIT_SHUTDOWN | task::BIT_ENABLED, true );
    (void)Task.setState( s );
    Task.entry = ++core::taskEntries;
    /*cstat -CERT-EXP39-C_d*/
    Task.pEventInfo = static_cast<_Event*>( this );
    /*cstat +CERT-EXP39-C_d*/
    return waitingList.insert( &Task, AT_BACK );
}
/*============================================================================*/
#if ( Q_FSM == 1 )
/*cstat -MISRAC++2008-7-1-2*/
static void fsmTaskCallback( event_t e )
{
    /*cstat -CERT-EXP36-C_b*/
    stateMachine * const sm = static_cast<stateMachine*>( e.self().getAttachedObject() );
    /*cstat +CERT-EXP36-C_b*/
    const sm::signal_t sig;
    (void)sm->run( sig );
    Q_UNUSED(e);
} 
/*cstat +MISRAC++2008-7-1-2*/
/*============================================================================*/
bool core::addStateMachineTask( task &Task, stateMachine &m, const priority_t p, const qOS::time_t t, const taskState s, void *arg ) noexcept
{
    bool retValue = core::addTask( Task, fsmTaskCallback, p, t, task::PERIODIC, s, arg );

    if ( retValue ) {
        Task.aObj = &m;
        m.owner = &Task;
        #if ( Q_QUEUES == 1 )
            if ( nullptr != m.sQueue ) {
                retValue = Task.attachQueue( *m.sQueue, queueLinkMode::QUEUE_COUNT, 1u );
            }
        #endif
    }
    return retValue;
}
#endif /*Q_FSM*/
/*============================================================================*/
#if ( Q_CLI == 1 )
static void cliTaskCallback( event_t e )
{
    /*cstat -CERT-EXP36-C_b*/
    commandLineInterface * const c = static_cast<commandLineInterface*>( e.self().getAttachedObject() ); 
    /*cstat +CERT-EXP36-C_b*/
    c->setData( &e );
    (void)c->run();
}
/*============================================================================*/
static void cliNotifyFcn( commandLineInterface *cli )
{
    /*cstat -CERT-EXP36-C_b*/
    (void)qOS::os.notify( notifyMode::SIMPLE, *static_cast<task*>( cli->getOwner() ), nullptr );
    /*cstat +CERT-EXP36-C_b*/
}
/*============================================================================*/
bool core::addCommandLineInterfaceTask( task &Task, commandLineInterface &cli, const priority_t p, void *arg ) noexcept
{
    bool retValue = false;

    if ( addEventTask( Task, cliTaskCallback, p, arg ) ) {
        Task.aObj = &cli;
        cli.owner = &Task;
        cli.xNotifyFcn = &cliNotifyFcn;
        retValue = true;
    }

    return retValue;
}
#endif /*Q_CLI*/
/*============================================================================*/
/*cstat -MISRAC++2008-7-1-2*/
bool core::setIdleTask( taskFcn_t callback ) noexcept
{
    return idle.setCallback( callback );
}
/*cstat +MISRAC++2008-7-1-2*/
/*============================================================================*/
bool core::schedulerRelease( void ) noexcept
{
    bits::multipleSet( flag, BIT_RELEASE_SCHED );
    return true;
}
/*cstat -MISRAC++2008-7-1-2*/
/*============================================================================*/
bool core::setSchedulerReleaseCallback( taskFcn_t callback ) noexcept
{
    bool retValue = false;

    if ( callback != releaseSchedCallback ) {
        releaseSchedCallback = callback;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool core::removeTask( task &Task ) noexcept
{
    Task.setFlags( task::BIT_REMOVE_REQUEST, true );
    return true;
}
/*cstat +MISRAC++2008-7-1-2*/
/*============================================================================*/
void core::triggerReleaseSchedEvent( void ) noexcept
{
    bits::multipleClear( flag, BIT_INIT );
    bits::multipleClear( flag, BIT_RELEASE_SCHED );
    _Event::FirstCall = ( false == bits::multipleGet( flag, BIT_FCALL_RELEASED ) );
    _Event::Trigger = trigger::bySchedulingRelease;
    _Event::TaskData = nullptr;
    if ( nullptr != releaseSchedCallback ) {
        const taskFcn_t callback = releaseSchedCallback;
        /*some low-end compilers cant deal with function-pointers inside structs*/
        /*cstat -CERT-EXP39-C_d*/
        callback( *static_cast<_Event*>( this) );
        /*cstat +CERT-EXP39-C_d*/
    }
    bits::multipleSet( flag, BIT_FCALL_IDLE );
}
/*============================================================================*/
bool core::checkIfReady( void ) noexcept
{
    bool xReady = false;
    #if ( Q_QUEUES == 1 )
        trigger trg;
    #endif
    task* xTask;

    #if ( Q_PRIO_QUEUE_SIZE > 0 )
        /*try to extract a task from the front of the priority queue*/
        xTask = priorityQueue.get();
        if ( nullptr  != xTask ) {  /*got a task from the priority queue?*/
            xTask->Trigger = trigger::byNotificationQueued;
            xTask->setFlags( task::BIT_SHUTDOWN, true ); /*wake-up the task!!*/
        }
    #endif

    for( auto i = waitingList.begin() ; i.until() ; i++ ) {
        xTask = i.get<task*>();
        
        if ( notifyMode::_NONE_ != nSpreader.mode ) {
            (void)notify( nSpreader.mode, *xTask, nSpreader.eventData );
            break;
        }

        if ( xTask->getFlag( task::BIT_SHUTDOWN ) ) {
            #if ( Q_PRIO_QUEUE_SIZE > 0 )
            if ( trigger::byNotificationQueued == xTask->Trigger ) {
                xReady = true;
            }
            else
            #endif
            if ( xTask->deadLineReached() ) {
                xTask->time.reload();
                xTask->Trigger = trigger::byTimeElapsed;
                xReady = true;
            }
            #if ( Q_QUEUES == 1 )
            /*cstat -MISRAC++2008-6-2-1*/
            else if ( trigger::None != ( trg = xTask->queueCheckEvents() ) ) {
                xTask->Trigger = trg;
                xReady = true;
            }
            /*cstat +MISRAC++2008-6-2-1*/
            #endif
            else if ( xTask->notifications > 0u ) {
                xTask->Trigger = trigger::byNotificationSimple;
                xReady = true;
            }
            else if ( 0uL != ( task::EVENT_FLAGS_MASK & xTask->flags ) ) {
                xTask->Trigger = trigger::byEventFlags;
                xReady = true;
            }
            else {
                xTask->Trigger = trigger::None;
                /*task has no available events, put it into a suspended state*/
            }
        }
        (void)waitingList.remove( nullptr, listPosition::AT_FRONT );
        if ( xTask->getFlag( task::BIT_REMOVE_REQUEST ) ) {
            #if ( Q_PRIO_QUEUE_SIZE > 0 )
                critical::enter();
                /*clean any entry of this task from the priority queue */
                priorityQueue.cleanUp( *xTask );
                critical::exit();
            #endif
            xTask->setFlags( task::BIT_REMOVE_REQUEST, false );
        }
        else {
            list * const xList = ( trigger::None != xTask->Trigger ) ? &coreLists[ xTask->priority ] : &suspendedList;
            (void)xList->insert( xTask, listPosition::AT_BACK );
        }
    }
    /*spread operation done, clean-up*/
    nSpreader.mode = notifyMode::_NONE_;
    nSpreader.eventData = nullptr;

    return xReady;
}
/*============================================================================*/
void core::dispatchTaskFillEventInfo( task *Task ) noexcept
{
    switch ( Task->Trigger ) {
        case trigger::byTimeElapsed: {
                const iteration_t taskIteration = Task->iterations;

                _Event::FirstIteration = ( ( task::PERIODIC != taskIteration ) && ( taskIteration < 0 ) );
                Task->iterations = ( _Event::FirstIteration ) ? -Task->iterations : Task->iterations;
                if ( task::PERIODIC  != Task->iterations ) {
                    --Task->iterations;
                }
                _Event::LastIteration = ( 0 == Task->iterations );
                if ( _Event::LastIteration ) {
                    /*When iteration value is reached, the task will be disabled*/
                    Task->setFlags( task::BIT_ENABLED, false );
                }
                _Event::StartDelay = Task->time.elapsed();
                break;
            }
        case trigger::byNotificationSimple:
            _Event::EventData = Task->asyncData; /*Transfer async-data to the eventInfo structure*/
            --Task->notifications;
            break;
        #if ( Q_QUEUES == 1 )
            case trigger::byQueueReceiver:
                _Event::EventData = Task->aQueue->peek(); /*the EventData will point to the queue front-data*/
                break;
            case trigger::byQueueFull: case trigger::byQueueCount: case trigger::byQueueEmpty: // skipcq: CXX-C1001
                /*the EventData will point to the the linked queue*/
                _Event::EventData = static_cast<void*>( Task->aQueue );
                break;
        #endif
        #if ( Q_PRIO_QUEUE_SIZE > 0 )
            case trigger::byNotificationQueued:
                /*get the extracted data from queue*/
                _Event::EventData = priorityQueue.data;
                priorityQueue.data = nullptr;
                break;
        #endif
            case trigger::byEventFlags:
                break;
            default: break;
    }
    /*Fill the event info structure: Trigger, FirstCall and TaskData */
    _Event::Trigger = Task->Trigger;
    _Event::FirstCall = false == Task->getFlag( task::BIT_INIT );
    _Event::TaskData = Task->taskData;
    _Event::currentTask = Task;
    //currentTask = Task;
}
/*============================================================================*/
void core::dispatch( list * const xList ) noexcept
{
    for ( auto i = xList->begin() ; i.until() ; i++ ) {
        task * const xTask = i.get<task*>();
        taskFcn_t taskActivities = xTask->callback;

        dispatchTaskFillEventInfo( xTask );
        yieldTask = nullptr;

        if ( nullptr != taskActivities ) {
            /*cstat -CERT-EXP39-C_d*/
            taskActivities( *static_cast<_Event*>( this ) );
            /*cstat +CERT-EXP39-C_d*/
        }

        while ( nullptr != yieldTask ) {
            _Event::currentTask = yieldTask;
            taskActivities = _Event::currentTask->callback;
            yieldTask = nullptr;
            if ( nullptr != taskActivities ) {
                /*yielded task inherits eventData*/
                /*cstat -CERT-EXP39-C_d*/
                taskActivities( *static_cast<_Event*>( this ) );
                /*cstat +CERT-EXP39-C_d*/
            }
        }
        
        currentTask = nullptr;
        (void)xList->remove( nullptr, listPosition::AT_FRONT );
        (void)waitingList.insert( xTask, listPosition::AT_BACK );
        #if ( Q_QUEUES == 1 )
            if ( trigger::byQueueReceiver == xTask->Trigger ) {
                /*remove the data from the attached Queue*/
                (void)xTask->aQueue->removeFront();
            }
        #endif
        /*set the init flag*/
        xTask->setFlags( task::BIT_INIT, true );
        _Event::FirstIteration = false;
        _Event::LastIteration = false;
        _Event::StartDelay = 0uL;
        _Event::EventData = nullptr; /*clear the eventData*/
        ++xTask->cycles; /*increase the task-cycles value*/
        xTask->Trigger = trigger::None;
    }
}
/*============================================================================*/
void core::dispatchIdle( void ) noexcept
{
    _Event::FirstCall = ( false == bits::multipleGet( flag, BIT_FCALL_IDLE ) );
    _Event::TaskData = nullptr;
    _Event::Trigger = trigger::byNoReadyTasks;
    _Event::currentTask = &idle;
    /*cstat -CERT-EXP39-C_d*/
    idle.callback( *static_cast<_Event*>( this ) ); /*run the idle callback*/
    /*cstat +CERT-EXP39-C_d*/
    bits::multipleSet( flag, BIT_FCALL_IDLE );
}
/*============================================================================*/
bool core::run( void ) noexcept
{
    /*cstat -MISRAC++2008-0-1-6*/
    bool retValue = false;
    /*cstat +MISRAC++2008-0-1-6*/

    do {
        if ( checkIfReady() ) {
            priority_t xPriorityListIndex = MAX_PRIORITY_VALUE;

            do {
                list* const xList = &coreLists[ xPriorityListIndex ];
                if ( xList->length() > 0u ) {
                    dispatch( xList );
                }
            } while( 0u != xPriorityListIndex-- );
        }
        else {
            if ( nullptr != idle.callback ) {
                dispatchIdle();
            }
        }
        if ( suspendedList.length() > 0u ) {
            (void)waitingList.move( suspendedList, listPosition::AT_BACK );
            #if ( Q_PRESERVE_TASK_ENTRY_ORDER == 1 )
                /*preseve the entry order by sorting the new waiting-list*/
                (void)waitingList.sort( taskEntryOrderPreserver );
            #endif
        }
    }
    #if ( Q_ALLOW_SCHEDULER_RELEASE == 1 )
        while ( false == bits::multipleGet( flag, BIT_RELEASE_SCHED ) );
        triggerReleaseSchedEvent(); /*check for a scheduling-release request*/
        retValue = true;
    #else
        while ( qTrue == qTrue );
    #endif

    return retValue;
}
/*============================================================================*/
#if ( Q_PRESERVE_TASK_ENTRY_ORDER == 1 )
static bool taskEntryOrderPreserver( const void *n1, const void *n2 )
{
    /*cstat -CERT-EXP36-C_b*/
    const task * const t1 = static_cast<const task*>( n1 );
    const task * const t2 = static_cast<const task*>( n2 );
    /*cstat +CERT-EXP36-C_b*/
    return ( t1->getID() > t2->getID() );
}
#endif
/*============================================================================*/
bool core::notify( notifyMode mode, task &Task, void* eventData ) noexcept
{
    bool retValue = false;

    if ( notifyMode::SIMPLE == mode ) {
        if ( Task.notifications < MAX_NOTIFICATION_VALUE ) {
            ++Task.notifications;
            Task.asyncData = eventData;
            retValue = true;
        }
    }
    else if ( notifyMode::QUEUED == mode ) {
        #if ( Q_PRIO_QUEUE_SIZE > 0 )
            retValue = priorityQueue.insert( Task, eventData );
        #endif
    }
    else {
        /*nothing to do here*/
    }

    return retValue;
}
/*============================================================================*/
bool core::notify( notifyMode mode, void* eventData ) noexcept
{
    bool retValue = false;
    /*do not proceed if any previous operation is in progress*/
    if ( notifyMode::_NONE_ == nSpreader.mode ) {
        if ( ( notifyMode::SIMPLE == mode ) || ( notifyMode::QUEUED == mode ) ) {
            nSpreader.mode = mode;
            nSpreader.eventData = eventData;
            retValue = true;
        }
    }
    return retValue;
}
/*============================================================================*/
bool core::hasPendingNotifications( task &Task ) noexcept
{
    /*cstat -MISRAC++2008-0-1-6*/
    bool retValue = false;
    /*cstat +MISRAC++2008-0-1-6*/
    if ( Task.notifications > 0u ) {
        retValue = true;
    }
    else {
        #if ( Q_PRIO_QUEUE_SIZE > 0 )
            retValue = priorityQueue.isTaskInside( Task );
        #endif
    }

    return retValue;
}
/*cstat -MISRAC++2008-7-1-2*/
/*============================================================================*/
bool core::eventFlagsModify( task &Task, const taskFlag_t tFlags, const bool action ) noexcept
{
    const taskFlag_t flagsToSet = tFlags & task::EVENT_FLAGS_MASK;
    Task.setFlags( flagsToSet, action );

    return true;
}
/*============================================================================*/
taskFlag_t core::eventFlagsRead( task &Task ) const noexcept
{
    return Task.flags & task::EVENT_FLAGS_MASK;
}
/*cstat +MISRAC++2008-7-1-2*/
/*============================================================================*/
bool core::eventFlagsCheck( task &Task, taskFlag_t flagsToCheck, const bool clearOnExit, const bool checkForAll ) noexcept
{
    bool retValue = false;
    const taskFlag_t cEventBits = Task.flags & task::EVENT_FLAGS_MASK;

    flagsToCheck &= task::EVENT_FLAGS_MASK;
    if ( false == checkForAll ) {
        if ( 0u != ( cEventBits & flagsToCheck ) ) {
            retValue = true;
        }
    }
    else {
        if ( ( cEventBits & flagsToCheck ) == flagsToCheck ) {
            retValue = true;
        }
    }
    if ( ( true == retValue ) && ( true == clearOnExit ) ) {
        Task.flags &= ~flagsToCheck;
    }

    return retValue;
}
/*============================================================================*/
task* core::findTaskByName( const char *name ) noexcept
{
    task *found = nullptr;

    if ( nullptr != name ) {
        const size_t maxLists = sizeof( coreLists )/sizeof( coreLists[ 0 ] );
        bool r = false;

        for ( size_t i = 0u ; ( false == r ) && ( i < maxLists ) ; ++i ) {
            for ( auto it = coreLists[ i ].begin() ; it.until() ; it++ ) {
                task * const xTask = it.get<task*>();

                if ( nullptr != xTask->name ) {
                    if ( 0 == strncmp( name, xTask->name, 32u ) ) {
                        found  = xTask;
                        r = true;
                        break;
                    }
                }
            }
        }
    }

    return found;
}
/*============================================================================*/
bool core::yieldToTask( task &Task ) noexcept
{
    bool  retValue = false;

    if ( ( nullptr != currentTask ) && ( &Task != currentTask ) ) {
        yieldTask = &Task;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
globalState core::getGlobalState( task &Task ) const noexcept
{
    globalState retValue = globalState::UNDEFINED;
    /*cstat -CERT-EXP36-C_b*/
    list * const xList = Task.getContainer();
    /*cstat +CERT-EXP36-C_b*/
    if ( currentTask == &Task ) {
        retValue = globalState::RUNNING;
    }
    else if ( &waitingList == xList ) {
        retValue = globalState::WAITING;
    }
    else if ( &suspendedList == xList ) {
        retValue = globalState::SUSPENDED;
    }
    else if ( nullptr == xList ) {
        /*undefined*/
    }
    else {
        retValue = globalState::READY; /*by discard, it must be ready*/
    }

    return retValue;
}
/*============================================================================*/
