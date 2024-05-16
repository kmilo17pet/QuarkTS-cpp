#include "include/kernel.hpp"
#include "include/critical.hpp"
#include "include/helper.hpp"

namespace qOS {
    core& os = core::getInstance(); // skipcq: CXX-W2011, CXX-W2009
}

using namespace qOS;

static bool taskEntryOrderPreserver( const void *n1, const void *n2 );

const priority_t core::MAX_PRIORITY_VALUE = static_cast<priority_t>( Q_PRIORITY_LEVELS ) - 1U;
const uint32_t core::BIT_INIT = 0x00000001UL;
const uint32_t core::BIT_FCALL_IDLE = 0x00000002UL;
const uint32_t core::BIT_RELEASE_SCHED = 0x00000004UL;
const uint32_t core::BIT_FCALL_RELEASED = 0x00000008UL;

const priority_t core::LOWEST_PRIORITY = 0U;
const priority_t core::MEDIUM_PRIORITY = static_cast<priority_t>( Q_PRIORITY_LEVELS ) >> 1U;
const priority_t core::HIGHEST_PRIORITY = static_cast<priority_t>( Q_PRIORITY_LEVELS ) - 1U;
const notifier_t MAX_NOTIFICATION_VALUE = UINT32_MAX - 1UL;

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
    (void)setNameIdleTask( "idle" );
    (void)idle.setPriority( core::LOWEST_PRIORITY );
    (void)idle.setState( taskState::DISABLED_STATE );
    (void)idle.setCallback( callbackIdle );
    idle.entry = 0U;
}
/*cstat +MISRAC++2008-7-1-2*/
/*============================================================================*/
bool core::addTask( task &Task, taskFcn_t callback, const priority_t p, const qOS::duration_t t, const iteration_t n, const qOS::taskState s, void *arg ) noexcept
{
    (void)Task.setCallback( callback );
    (void)Task.time.set( t );
    (void)Task.setData( arg );
    (void)Task.setPriority( clipUpper( p, MAX_PRIORITY_VALUE ) );
    (void)Task.setIterations( n );
    Task.setFlags( task::BIT_SHUTDOWN | task::BIT_ENABLED, true );
    (void)Task.setState( s );
    Task.entry = ++core::taskEntries;
    return waitingList.insert( &Task, AT_BACK );
}
/*============================================================================*/
#if ( Q_FSM == 1 )
/*cstat -MISRAC++2008-7-1-2*/
static void fsmTaskCallback( event_t e )
{
    /*cstat -CERT-EXP36-C_b*/
    stateMachine * const sm = static_cast<stateMachine*>( e.thisTask().getAttachedObject() );
    /*cstat +CERT-EXP36-C_b*/
    const sm::signal_t sig;
    (void)sm->run( sig );
    Q_UNUSED(e);
}
/*cstat +MISRAC++2008-7-1-2*/
/*============================================================================*/
bool core::addStateMachineTask( task &Task, stateMachine &m, const priority_t p, const qOS::duration_t t, const taskState s, void *arg ) noexcept
{
    bool retValue = core::addTask( Task, fsmTaskCallback, p, t, task::PERIODIC, s, arg );

    if ( retValue ) {
        Task.aObj = &m;
        m.owner = &Task;
        #if ( Q_QUEUES == 1 )
            if ( nullptr != m.sQueue ) {
                retValue = Task.attachQueue( *m.sQueue, queueLinkMode::QUEUE_COUNT, 1U );
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
    commandLineInterface * const c = static_cast<commandLineInterface*>( e.thisTask().getAttachedObject() );
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
bool core::setNameIdleTask( const char *tName ) noexcept
{
    bool retValue = false;
    const size_t nl = util::strlen( tName , sizeof(idle.name) );
    /*cstat -MISRAC++2008-5-14-1*/
    if ( ( nl > 0U ) && ( nl < sizeof(idle.name) ) ) {
        (void)util::strcpy( idle.name, tName , sizeof( idle.name ) ); // skipcq: CXX-C1000
        retValue = true;

    }
    /*cstat +MISRAC++2008-5-14-1*/
    return retValue;
}
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
    taskEvent::FirstCall = !bits::multipleGet( flag, BIT_FCALL_RELEASED );
    taskEvent::Trigger = trigger::bySchedulingRelease;
    taskEvent::TaskData = nullptr;
    if ( nullptr != releaseSchedCallback ) {
        const taskFcn_t callback = releaseSchedCallback;
        /*some low-end compilers cant deal with function-pointers inside structs*/
        /*cstat -CERT-EXP39-C_d*/
        callback( *static_cast<taskEvent*>( this) );
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

    for( auto i = waitingList.begin() ; i.untilEnd() ; i++ ) {
        xTask = i.get<task*>();

        if ( notifyMode::NOTIFY_NONE != nSpreader.mode ) {
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
            else if ( xTask->notifications > 0U ) {
                xTask->Trigger = trigger::byNotificationSimple;
                xReady = true;
            }
            else if ( 0UL != ( task::EVENT_FLAGS_MASK & xTask->flags ) ) {
                xTask->Trigger = trigger::byEventFlags;
                xReady = true;
            }
            else {
                xTask->Trigger = trigger::None;
                /*task has no available events, put it into a suspended state*/
            }
        }
        (void)waitingList.remove( listPosition::AT_FRONT );
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
    nSpreader.mode = notifyMode::NOTIFY_NONE;
    nSpreader.eventData = nullptr;

    return xReady;
}
/*============================================================================*/
void core::dispatchTaskFillEventInfo( task *Task ) noexcept
{
    switch ( Task->Trigger ) {
        case trigger::byTimeElapsed: {
                const iteration_t taskIteration = Task->iterations;

                taskEvent::FirstIteration = ( ( task::PERIODIC != taskIteration ) && ( taskIteration < 0 ) );
                Task->iterations = ( taskEvent::FirstIteration ) ? -Task->iterations : Task->iterations;
                if ( task::PERIODIC  != Task->iterations ) {
                    --Task->iterations;
                }
                taskEvent::LastIteration = ( 0 == Task->iterations );
                if ( taskEvent::LastIteration ) {
                    /*When iteration value is reached, the task will be disabled*/
                    Task->setFlags( task::BIT_ENABLED, false );
                }
                taskEvent::StartDelay = Task->time.elapsed();
                break;
            }
        case trigger::byNotificationSimple:
            taskEvent::EventData = Task->asyncData; /*Transfer async-data to the eventInfo structure*/
            --Task->notifications;
            break;
        #if ( Q_QUEUES == 1 )
            case trigger::byQueueReceiver:
                taskEvent::EventData = Task->aQueue->peek(); /*the EventData will point to the queue front-data*/
                break;
            case trigger::byQueueFull: case trigger::byQueueCount: case trigger::byQueueEmpty: // skipcq: CXX-C1001
                /*the EventData will point to the the linked queue*/
                taskEvent::EventData = static_cast<void*>( Task->aQueue );
                break;
        #endif
        #if ( Q_PRIO_QUEUE_SIZE > 0 )
            case trigger::byNotificationQueued:
                /*get the extracted data from queue*/
                taskEvent::EventData = priorityQueue.data;
                priorityQueue.data = nullptr;
                break;
        #endif
            default:
                break;
    }
    /*Fill the event info structure: Trigger, FirstCall and TaskData */
    taskEvent::Trigger = Task->Trigger;
    taskEvent::FirstCall = !Task->getFlag( task::BIT_INIT );
    taskEvent::TaskData = Task->taskData;
    taskEvent::currentTask = Task;
    //currentTask = Task;
}
/*============================================================================*/
void core::dispatch( list * const xList ) noexcept
{
    for ( auto i = xList->begin() ; i.untilEnd() ; i++ ) {
        task * const xTask = i.get<task*>();
        /*cstat -CERT-EXP39-C_d*/
        event_t e = *( static_cast<taskEvent*>( this ) );
        /*cstat +CERT-EXP39-C_d*/
        dispatchTaskFillEventInfo( xTask );
        yieldTask = nullptr;
        xTask->activities( e );
        /*cppcheck-suppress knownConditionTrueFalse */
        while ( nullptr != yieldTask ) {
            taskEvent::currentTask = yieldTask;
            yieldTask = nullptr;
            taskEvent::currentTask->activities( e );
        }

        currentTask = nullptr;
        (void)xList->remove( listPosition::AT_FRONT );
        (void)waitingList.insert( xTask, listPosition::AT_BACK );
        #if ( Q_QUEUES == 1 )
            if ( trigger::byQueueReceiver == xTask->Trigger ) {
                (void)xTask->aQueue->removeFront(); /*remove the data from the attached Queue*/
            }
        #endif
        xTask->setFlags( task::BIT_INIT, true );
        taskEvent::FirstIteration = false;
        taskEvent::LastIteration = false;
        taskEvent::StartDelay = 0UL;
        taskEvent::EventData = nullptr; /*clear the eventData*/
        ++xTask->cycles; /*increase the task-cycles value*/
        xTask->Trigger = trigger::None;
    }
}
/*============================================================================*/
void core::dispatchIdle( void ) noexcept
{
    /*cstat -CERT-EXP39-C_d*/
    event_t e = *( static_cast<taskEvent*>( this ) );
    /*cstat +CERT-EXP39-C_d*/
    taskEvent::FirstCall = !bits::multipleGet( flag, BIT_FCALL_IDLE );
    taskEvent::TaskData = nullptr;
    taskEvent::Trigger = trigger::byNoReadyTasks;
    taskEvent::currentTask = &idle;
    idle.activities( e );
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
                if ( xList->length() > 0U ) {
                    dispatch( xList );
                }
            } while( 0U != xPriorityListIndex-- );
        }
        else {
            if ( nullptr != idle.callback ) {
                dispatchIdle();
            }
        }
        if ( inputWatchers.length() > 0U ) {
            handleInputWatchers();
        }
        if ( suspendedList.length() > 0U ) {
            (void)waitingList.move( suspendedList, listPosition::AT_BACK );
            #if ( Q_PRESERVE_TASK_ENTRY_ORDER == 1 )
                /*preseve the entry order by sorting the new waiting-list*/
                (void)waitingList.sort( taskEntryOrderPreserver );
            #endif
        }
    }
    #if ( Q_ALLOW_SCHEDULER_RELEASE == 1 )
        while ( !bits::multipleGet( flag, BIT_RELEASE_SCHED ) );
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

    if ( &Task != &idle ) { /*idle task cannot be notified*/
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
    }
    return retValue;
}
/*============================================================================*/
bool core::notify( notifyMode mode, void* eventData ) noexcept
{
    bool retValue = false;
    /*do not proceed if any previous operation is in progress*/
    if ( notifyMode::NOTIFY_NONE == nSpreader.mode ) {
        if ( ( notifyMode::SIMPLE == mode ) || ( notifyMode::QUEUED == mode ) ) {
            nSpreader.mode = mode;
            nSpreader.eventData = eventData;
            retValue = true;
        }
    }
    return retValue;
}
/*============================================================================*/
bool core::hasPendingNotifications( const task &Task ) const noexcept
{
    /*cstat -MISRAC++2008-0-1-6*/
    bool retValue = false;
    /*cstat +MISRAC++2008-0-1-6*/
    if ( Task.notifications > 0U ) {
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
    if ( !checkForAll ) {
        if ( 0U != ( cEventBits & flagsToCheck ) ) {
            retValue = true;
        }
    }
    else {
        if ( ( cEventBits & flagsToCheck ) == flagsToCheck ) {
            retValue = true;
        }
    }

    if ( ( retValue ) && ( clearOnExit ) ) {
        Task.flags &= ~flagsToCheck;
    }

    return retValue;
}
/*============================================================================*/
task* core::getTaskByName( const char *name ) noexcept
{
    task *found = nullptr;

    if ( nullptr != name ) {
        const size_t maxLists = sizeof( coreLists )/sizeof( coreLists[ 0 ] );
        bool r = false;

        for ( size_t i = 0U ; ( !r ) && ( i < maxLists ) ; ++i ) {
            for ( auto it = coreLists[ i ].begin() ; it.untilEnd() ; it++ ) {
                task * const xTask = it.get<task*>();

                if ( 0 == strncmp( name, xTask->name, sizeof(xTask->name) - 1U ) ) {  // skipcq: CXX-C1000
                    found  = xTask;
                    r = true;
                    break;
                }

            }
        }
    }

    return found;
}
/*============================================================================*/
task* core::getTaskByID( size_t id ) noexcept
{
    task *found = nullptr;

    if ( ( id > 0U ) && ( id < SIZE_MAX ) ) {
        const size_t maxLists = sizeof( coreLists )/sizeof( coreLists[ 0 ] );
        bool r = false;

        for ( size_t i = 0U ; ( !r ) && ( i < maxLists ) ; ++i ) {
            for ( auto it = coreLists[ i ].begin() ; it.untilEnd() ; it++ ) {
                task * const xTask = it.get<task*>();

                if ( id == xTask->entry ) {
                    found  = xTask;
                    r = true;
                    break;
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
bool core::addInputWatcher( input::watcher &w ) noexcept
{
    return inputWatchers.insert( &w );
}
/*============================================================================*/
bool core::removeInputWatcher( input::watcher &w ) noexcept
{
    return inputWatchers.remove( &w );
}
/*============================================================================*/
void core::handleInputWatchers( void ) noexcept
{
    for ( auto i = inputWatchers.begin(); i.untilEnd() ; i++ ) {
        input::watcher * const w = i.get<input::watcher*>();
        (void)w->watch();
    }
}