#include "kernel.hpp"
#include "critical.hpp"
#include "helper.hpp"

namespace qOS {
    core& os = core::getInstance();
}

using namespace qOS;

static bool taskEntryOrderPreserver( listCompareHandle_t h );

const priority_t core::LOWEST_PRIORITY = 0u;
const priority_t core::MEDIUM_PRIORITY = Q_PRIORITY_LEVELS >> 1u;
const priority_t core::HIGHEST_PRIORITY = Q_PRIORITY_LEVELS - 1u;

const notifier_t MAX_NOTIFICATION_VALUE = UINT32_MAX - 1uL;

static void fsmTaskCallback( event_t e );

/*============================================================================*/
core& core::getInstance( void )
{
    static core instance;
    return instance;
}
/*============================================================================*/
void core::init( const getTickFcn_t tFcn, const timingBase_t t, taskFcn_t idleCallback )
{
    this->idleCallback = idleCallback;
    (void)clock::setTimeBase( t );
    (void)clock::setTickProvider( tFcn );
    
}
/*============================================================================*/
bool core::addTask( task *Task, taskFcn_t callback, const priority_t p, const qOS::time_t t, const iteration_t n, const taskState init, void *arg )
{
    bool retValue = false;

    if ( nullptr != Task ) {
        Task->setCallback( callback );
        Task->time.set( t );
        Task->setData( arg );
        Task->setPriority( clipUpper( p, MAX_PRIORITY_VALUE ) );
        Task->setIterations( n );
        Task->setFlags( task::BIT_SHUTDOWN | task::BIT_ENABLED, true );
        Task->setState( init );
        Task->entry = core::taskEntries++;
        Task->pEventInfo = static_cast<_Event*>( this );
        retValue = waitingList->insert( Task, AT_BACK );
    }

    return retValue;
}
/*============================================================================*/
bool core::addEventTask( task *Task, taskFcn_t callback, const priority_t p, void *arg )
{
    return core::addTask( Task, callback, p, clock::IMMEDIATE, task::SINGLE_SHOT, taskState::DISABLED, arg );
}
/*============================================================================*/
static void fsmTaskCallback( event_t e )
{
    task *xTask = qOS::os.getTaskRunning();
    stateMachine *sm = static_cast<stateMachine*>( xTask->getAttachedObject() );
    fsm::signal_t sig = { fsm::signalID::SIGNAL_NONE, nullptr };
    (void)sm->run( sig );
} 
/*============================================================================*/
bool core::addStateMachineTask( task *Task, stateMachine *m, const priority_t p, const qOS::time_t t, const taskState init, void *arg )
{
    bool retValue = core::addTask( Task, fsmTaskCallback, p, t, task::PERIODIC, init, arg );
    if ( retValue ) {
        Task->aObj = m;
        m->owner = Task;
        #if ( Q_QUEUES == 1 )
            if ( nullptr != m->sQueue ) {
                /*bind the queue*/
                retValue =Task->attachQueue( m->sQueue, queueLinkMode::QUEUE_COUNT, 1u );
            }
        #endif
    }
    return retValue;
}
/*============================================================================*/
task* core::getTaskRunning( void ) const
{
    return currentTask;
}
/*============================================================================*/
bool core::setIdleTask( taskFcn_t callback )
{
    bool retValue = false;

    if ( callback != idleCallback ) {
        idleCallback = callback;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool core::schedulerRelease( void )
{
    bitsSet( flag, BIT_RELEASE_SCHED );
    return true;
}
/*============================================================================*/
bool core::setSchedulerReleaseCallback( taskFcn_t callback )
{
    bool retValue = false;

    if ( callback != releaseSchedCallback ) {
        releaseSchedCallback = callback;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool core::removeTask( task *Task )
{
    bool retValue = false;

    if ( nullptr != Task ) {
        Task->setFlags( task::BIT_REMOVE_REQUEST, true );
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
void core::triggerReleaseSchedEvent( void )
{
    bitsClear( flag, BIT_INIT );
    bitsClear( flag, BIT_RELEASE_SCHED );
    _Event::FirstCall = ( false == bitsGet( flag, BIT_FCALL_RELEASED ) );
    _Event::Trigger = bySchedulingRelease;
    _Event::TaskData = nullptr;
    if ( nullptr != releaseSchedCallback ) {
        taskFcn_t callback = releaseSchedCallback;
        /*some low-end compilers cant deal with function-pointers inside structs*/
        callback( *static_cast<_Event*>( this) );
    }
    bitsSet( flag, BIT_FCALL_IDLE );
}
/*============================================================================*/
bool core::checkIfReady( void )
{
    bool xReady = false;
    trigger trg;
    task* xTask;

    #if ( Q_PRIO_QUEUE_SIZE > 0 )
        /*try to extract a task from the front of the priority queue*/
        xTask = priorityQueue.get();
        if ( nullptr  != xTask ) {  /*got a task from the priority queue?*/
            xTask->Trigger = byNotificationQueued;
            xTask->setFlags( task::BIT_SHUTDOWN, true ); /*wake-up the task!!*/
        }
    #endif

    for( auto i = waitingList->begin() ; i.until() ; i++ ) {
        xTask = i.get<task*>();
        #if ( Q_NOTIFICATION_SPREADER == 1 )
            if ( nullptr != nSpreader.mode ) {
                nSpreader.mode( xTask, nSpreader.eventData );
                break;
            }
        #endif

        if ( xTask->getFlag( task::BIT_SHUTDOWN ) ) {
            #if ( Q_PRIO_QUEUE_SIZE > 0 )
            if ( byNotificationQueued == xTask->Trigger ) {
                xReady = true;
            }
            else
            #endif
            if ( xTask->deadLineReached() ) {
                xTask->time.reload();
                xTask->Trigger = byTimeElapsed;
                xReady = true;
            }
            #if ( Q_QUEUES == 1 )
            else if ( trigger::None != ( trg = xTask->queueCheckEvents() ) ) {
                xTask->Trigger = trg;
                xReady = true;
            }
            #endif
            else if ( xTask->notifications > 0u ) {
                xTask->Trigger = byNotificationSimple;
                xReady = true;
            }
            #if ( Q_TASK_EVENT_FLAGS == 1 )
            else if ( 0uL != ( task::EVENT_FLAGS_MASK & xTask->flags ) ) {
                xTask->Trigger = byEventFlags;
                xReady = true;
            }
            #endif
            else {
                xTask->Trigger = trigger::None;
                /*task has no available events, put it into a suspended state*/
            }
        }
        waitingList->remove( nullptr, listPosition::AT_FRONT );
        if ( xTask->getFlag( task::BIT_REMOVE_REQUEST ) ) {
            #if ( Q_PRIO_QUEUE_SIZE > 0 )
                critical::enter();
                /*clean any entry of this task from the priority queue */
                priorityQueue.cleanUp( xTask );
                critical::exit();
            #endif
            xTask->setFlags( task::BIT_REMOVE_REQUEST, false );
        }
        else {
            list *xList = ( trigger::None != xTask->Trigger ) ? &readyList[ xTask->priority ] : suspendedList;
            xList->insert( xTask, listPosition::AT_BACK );
        }
    }
    #if ( Q_NOTIFICATION_SPREADER == 1 )
        /*spread operation done, clean-up*/
        nSpreader.mode = nullptr;
        nSpreader.eventData = nullptr;
    #endif
    
    return xReady;
}
/*============================================================================*/
void core::dispatchTaskFillEventInfo( task *Task )
{
    trigger xEvent = Task->Trigger;
    iteration_t taskIteration;
    /*
    take the necessary actions before dispatching, depending on the event that
    triggered the task
    */
    switch ( xEvent ) {
        case byTimeElapsed:
            /*handle the iteration value and the FirstIteration flag*/
            taskIteration = Task->iterations;
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
        case byNotificationSimple:
            /*Transfer async-data to the eventInfo structure*/
            _Event::EventData = Task->asyncData;
            --Task->notifications;
            break;
        #if ( Q_QUEUES == 1 )
            case byQueueReceiver:
                /*the EventData will point to the queue front-data*/
                _Event::EventData = Task->aQueue->peek();
                break;
            case byQueueFull: case byQueueCount: case byQueueEmpty:
                /*the EventData will point to the the linked queue*/
                _Event::EventData = (void*)Task->aQueue;
                break;
        #endif
        #if ( Q_PRIO_QUEUE_SIZE > 0 )
            case byNotificationQueued:
                /*get the extracted data from queue*/
                _Event::EventData = priorityQueue.data;
                priorityQueue.data = nullptr;
                break;
        #endif
        #if ( Q_TASK_EVENT_FLAGS == 1 )
            case byEventFlags:
                break;
        #endif
            default: break;
    }
    /*Fill the event info structure: Trigger, FirstCall and TaskData */
    _Event::Trigger = Task->Trigger;
    _Event::FirstCall = false == Task->getFlag( task::BIT_INIT );
    _Event::TaskData = Task->taskData;
    currentTask = Task; /*needed for qTask_Self()*/
}
/*============================================================================*/
void core::dispatch( list *xList )
{
    for ( auto i = xList->begin() ; i.until() ; i++ ) {
        auto xTask = i.get<task*>();
        taskFcn_t taskActivities = xTask->callback;

        dispatchTaskFillEventInfo( xTask );
        #if ( Q_ALLOW_YIELD_TO_TASK == 1 )
            yieldTask = nullptr;
        #endif

        if ( nullptr != taskActivities ) {
            taskActivities( *static_cast<_Event*>( this ) );
        }

        #if ( Q_ALLOW_YIELD_TO_TASK == 1 )
            while ( nullptr != yieldTask ) {
                currentTask = yieldTask;
                taskActivities = currentTask->callback;
                yieldTask = nullptr;
                if ( nullptr != taskActivities ) {
                    /*yielded task inherits eventData*/
                    taskActivities( *static_cast<_Event*>( this ) );
                }
            }
        #endif

        currentTask = nullptr;
        xList->remove( nullptr, listPosition::AT_FRONT );
        waitingList->insert( xTask, listPosition::AT_BACK );
        #if ( Q_QUEUES == 1 )
            if ( byQueueReceiver == xTask->Trigger ) {
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
        #if ( Q_TASK_COUNT_CYCLES == 1 )
            ++xTask->cycles; /*increase the task-cycles value*/
        #endif
        xTask->Trigger = trigger::None;
    }
}
/*============================================================================*/
void core::dispatchIdle( void )
{
    _Event::FirstCall = ( false == bitsGet( flag, BIT_FCALL_IDLE ) );
    _Event::TaskData = nullptr;
    _Event::Trigger = trigger::byNoReadyTasks;
    idleCallback( *static_cast<_Event*>( this ) ); /*run the idle callback*/
    bitsSet( flag, BIT_FCALL_IDLE );
}
/*============================================================================*/
bool core::run( void )
{
    bool retValue = false;

    do {
        if ( checkIfReady() ) {
            priority_t xPriorityListIndex = MAX_PRIORITY_VALUE;

            do {
                list* xList = &readyList[ xPriorityListIndex ];
                if ( xList->length() > 0u ) {
                    dispatch( xList );
                }
            } while( 0u != xPriorityListIndex-- );
        }
        else {
            if ( nullptr != idleCallback ) {
                dispatchIdle();
            }
        }
        if ( suspendedList->length() > 0u ) {
            (void)waitingList->move( suspendedList, listPosition::AT_BACK );
            #if ( Q_PRESERVE_TASK_ENTRY_ORDER == 1 )
                /*preseve the entry order by sorting the new waiting-list*/
                (void)waitingList->sort( taskEntryOrderPreserver );
            #endif
        }
    }
    #if ( Q_ALLOW_SCHEDULER_RELEASE == 1 )
        while ( false == bitsGet( flag, BIT_RELEASE_SCHED ) );
        triggerReleaseSchedEvent(); /*check for a scheduling-release request*/
        retValue = true;
    #else
        while ( qTrue == qTrue );
    #endif

    return retValue;
}
/*============================================================================*/
#if ( Q_PRESERVE_TASK_ENTRY_ORDER == 1 )
static bool taskEntryOrderPreserver( listCompareHandle_t h )
{
    task *t1, *t2;
    t1 = static_cast<task*>( h->n1 );
    t2 = static_cast<task*>( h->n2 );
    return ( t1->getID() > t2->getID() );
}
#endif
/*============================================================================*/
bool core::notify( notifyMode mode, task* Task, void* eventData )
{
    bool retValue = false;

    if ( nullptr != Task ) {
        if ( notifyMode::SIMPLE == mode ) {
            if ( Task->notifications < MAX_NOTIFICATION_VALUE ) {
                ++Task->notifications;
                Task->asyncData = eventData;
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
bool core::hasPendingNotifications( task* Task )
{
    bool retValue = false;

    if ( nullptr != Task ) {
        if ( Task->notifications > 0u ) {
            retValue = true;
        }
        else {
            #if ( Q_PRIO_QUEUE_SIZE > 0 )
                retValue = priorityQueue.isTaskInside( Task );
            #endif
        }
    }

    return retValue;
}
/*============================================================================*/
bool core::eventFlagsModify( task* Task, const taskFlag_t flags, const bool action )
{
    bool retValue = false;

    if ( nullptr != Task ) {
        taskFlag_t flagsToSet = Task->flags & task::EVENT_FLAGS_MASK;
        Task->setFlags( flagsToSet, action );
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
taskFlag_t core::eventFlagsRead( task *Task ) const
{
    taskFlag_t retValue = 0u;

    if ( nullptr != Task ) {
        retValue = Task->flags & task::EVENT_FLAGS_MASK;
    }

    return retValue;
}
/*============================================================================*/
bool core::eventFlagsCheck( task *Task, taskFlag_t flagsToCheck, const bool clearOnExit, const bool checkForAll )
{
    bool retValue = false;

    if ( nullptr != Task ) {
        taskFlag_t cEventBits = Task->flags & task::EVENT_FLAGS_MASK;

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
            Task->flags &= ~flagsToCheck;
        }
    }

    return retValue;
}
/*============================================================================*/
task* core::findTaskByName( const char *name )
{
    task *found = nullptr;

    if ( nullptr != name ) {
        const size_t maxLists = sizeof( coreLists )/sizeof( coreLists[ 0 ] );
        bool r = false;

        for ( size_t i = 0u ; ( false == r ) && ( i < maxLists ) ; ++i ) {
            for ( auto it = coreLists[ i ].begin() ; it.until() ; it++ ) {
                auto xTask = it.get<task*>();
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
bool core::yieldToTask( task *Task )
{
    bool  retValue = false;

    if ( ( nullptr != currentTask ) && ( Task != currentTask ) ) {
        yieldTask = Task;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
globalState core::getGlobalState( task *Task ) const
{
    globalState retValue = UNDEFINED;

    if ( nullptr != Task ) {
        list *xList = static_cast<list*>( Task->container );
        if ( currentTask == Task ) {
            retValue = RUNNING;
        }
        else if ( waitingList == xList ) {
            retValue = WAITING;
        }
        else if ( suspendedList == xList ) {
            retValue = SUSPENDED;
        }
        else if ( nullptr == xList ) {
            /*undefined*/
        }
        else {
            retValue = READY; /*by discard, it must be ready*/
        }
    }

    return retValue;
}
/*============================================================================*/

