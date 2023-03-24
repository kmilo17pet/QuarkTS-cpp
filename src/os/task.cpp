#include "task.hpp"
#include "helper.hpp"

using namespace qOS;

iteration_t task::PERIODIC = INT32_MIN;
iteration_t task::INDEFINITE = task::PERIODIC;
iteration_t task::SINGLE_SHOT = 1;

const std::uint32_t task::BIT_INIT = 0x00000001uL;
const std::uint32_t task::BIT_ENABLED = 0x00000002uL;
const std::uint32_t task::BIT_QUEUE_RECEIVER = 0x00000004uL;
const std::uint32_t task::BIT_QUEUE_FULL = 0x00000008uL;
const std::uint32_t task::BIT_QUEUE_COUNT = 0x00000010uL;
const std::uint32_t task::BIT_QUEUE_EMPTY = 0x00000020uL;
const std::uint32_t task::BIT_SHUTDOWN = 0x00000040uL;
const std::uint32_t task::BIT_REMOVE_REQUEST = 0x00000080uL;

const std::uint32_t task::EVENT_FLAGS_MASK = 0xFFFFF000uL;
const std::uint32_t task::QUEUE_FLAGS_MASK = 0x0000003CuL;

#define TASK_ITER_VALUE( x )                                               \
( ( ( (x) < 0 ) && ( (x) != PERIODIC ) ) ? -(x) : (x) )                    \

/*============================================================================*/
void task::setFlags( const std::uint32_t flags, const bool value )
{
    if ( value ) {
        bitsSet( this->flags, flags );
    }
    else {
        bitsClear( this->flags, flags );
    }
}
/*============================================================================*/
bool task::getFlag( const std::uint32_t flag ) const
{
    std::uint32_t xBit;
    xBit = this->flags & flag;
    return ( 0uL != xBit );
}
/*============================================================================*/
priority_t task::getPriority( void ) const
{
    return priority;
}
/*============================================================================*/
bool task::setPriority( priority_t pValue )
{
    bool retValue = false;

    if ( pValue < static_cast<priority_t>( Q_PRIORITY_LEVELS ) ) {
        priority = pValue;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
cycles_t task::getCycles( void ) const
{
    return cycles;
}
/*============================================================================*/
taskState task::getState( void ) const
{
    taskState retValue = ASLEEP;

    retValue = static_cast<taskState>( getFlag( BIT_SHUTDOWN ) );
    if ( retValue ) { /*Task is awaken*/
        retValue = static_cast<taskState>( getFlag( BIT_ENABLED ) );
    }

    return retValue;
}
/*============================================================================*/
bool task::deadLineReached( void ) const
{
    bool retValue = false;

    if ( getFlag( BIT_ENABLED ) ) {
        iteration_t iters = iterations;
        /*task should be periodic or must have available iters*/
        if ( ( TASK_ITER_VALUE( iters ) > 0 ) || ( PERIODIC == iters ) ) {
            /*check the time deadline*/
            if ( ( 0uL == time.getInterval() ) || time.expired() ) {
                retValue = true;
            }
        }
    }

    return retValue;
}
/*============================================================================*/
bool task::setState( taskState s )
{
    bool retValue = false;

    switch ( s ) {
        case DISABLED: case ENABLED:
            if ( s != static_cast<taskState>( getFlag( BIT_ENABLED ) ) ) {
                setFlags( BIT_ENABLED, static_cast<bool>( s ) );
                time.reload();
            }
            retValue = true;
            break;
        case ASLEEP:
            setFlags( BIT_SHUTDOWN, false );
            retValue = true;
            break;
        case AWAKE:
            setFlags( BIT_SHUTDOWN, true );
            retValue = true;
            break;
        default:
            break;
    }

    return retValue;
}
/*============================================================================*/
void task::setIterations( iteration_t iValue )
{
    if ( iValue >= 0u ) {
        iterations = -iValue;
    }
    else if ( PERIODIC == iValue ) {
        iterations = PERIODIC;
    }
    else {
        /*nothing to do, return qFalse*/
    }
}
/*============================================================================*/
bool task::setTime( const qOS::time_t tValue )
{
    return time.set( tValue );
}
/*============================================================================*/
bool task::setCallback( taskFcn_t callback )
{
    bool retValue = false;

    if ( callback != this->callback ) {
        this->callback = callback;
        #if ( ( Q_FSM == 1 ) || ( Q_ATCLI == 1 ) )
            aObj = nullptr;
        #endif
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool task::setData( void *arg )
{
    bool retValue = false;

    if ( arg != taskData ) {
        taskData = arg;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool task::setName( const char *name )
{
    bool retValue = false;
    if ( nullptr != name ) {
        this->name = name;
    }
    return retValue;
}
/*============================================================================*/
const char* task::getName( void ) const
{
    return ( nullptr != name ) ? name : "nullptr";
}
/*============================================================================*/
trigger task::queueCheckEvents( void )
{
    trigger retValue = trigger::None;

    if ( nullptr != aQueue ) {
        bool fullFlag, countFlag, receiverFlag, emptyFlag;
        std::size_t qCount; /*current queue count*/

        fullFlag = getFlag( BIT_QUEUE_FULL );
        countFlag = getFlag( BIT_QUEUE_COUNT );
        receiverFlag = getFlag( BIT_QUEUE_RECEIVER );
        emptyFlag = getFlag( BIT_QUEUE_EMPTY );

        qCount = aQueue->count(); /*to avoid side effects*/
        /*check the queue events in the corresponding precedence order*/
        if ( fullFlag && aQueue->isFull() ) {
            retValue = trigger::byQueueFull;
        }
        else if ( ( countFlag ) && ( qCount >= aQueueCount ) ) {
            retValue = trigger::byQueueCount;
        }
        else if ( receiverFlag && ( qCount > 0u ) ) {
            retValue = trigger::byQueueReceiver;
        }
        else if ( emptyFlag && aQueue->isEmpty() ) {
            /*qQueue_IsEmpty is known to not have side effects*/
            retValue = trigger::byQueueEmpty;
        }
        else {
            /*this case does not need to be handled*/
        }
    }

    return retValue;
}
/*============================================================================*/
std::size_t task::getID( void ) const
{
    return entry;
}
/*============================================================================*/
bool task::attachQueue( queue &q, const queueLinkMode mode, const std::size_t arg )
{
    bool retValue = false;

    if ( q.isReady() ) {
        setFlags( static_cast<std::uint32_t>( mode ) & QUEUE_FLAGS_MASK, 0u != arg );
        if ( queueLinkMode::QUEUE_COUNT == mode ) {
            aQueueCount = arg;
        }
        aQueue = ( arg > 0u ) ? &q : nullptr;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
void* task::getAttachedObject( void ) const
{
    return aObj;
}
/*============================================================================*/
event_t task::eventData( void ) const
{
    return *pEventInfo;
}
/*============================================================================*/