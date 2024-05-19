#include "include/task.hpp"
#include "include/kernel.hpp"
#include "include/helper.hpp"
#include "include/util.hpp"

using namespace qOS;

const iteration_t task::PERIODIC = INT32_MIN;
const iteration_t task::INDEFINITE = task::PERIODIC;
const iteration_t task::SINGLE_SHOT = 1;

const uint32_t task::BIT_INIT = 0x00000001UL;
const uint32_t task::BIT_ENABLED = 0x00000002UL;
const uint32_t task::BIT_QUEUE_RECEIVER = 0x00000004UL;
const uint32_t task::BIT_QUEUE_FULL = 0x00000008UL;
const uint32_t task::BIT_QUEUE_COUNT = 0x00000010UL;
const uint32_t task::BIT_QUEUE_EMPTY = 0x00000020UL;
const uint32_t task::BIT_SHUTDOWN = 0x00000040UL;
const uint32_t task::BIT_REMOVE_REQUEST = 0x00000080UL;
const uint32_t task::EVENT_FLAGS_MASK = 0xFFFFF000UL;
const uint32_t task::QUEUE_FLAGS_MASK = 0x0000003CUL;

/*============================================================================*/
constexpr iteration_t TASK_ITER_VALUE( iteration_t x )
{
    return ( ( x < 0 ) && ( x != task::PERIODIC ) ) ? -x : x;
}
/*============================================================================*/
void task::activities( event_t e )
{
    if ( nullptr != callback ) {
        callback( e );
    }
}
/*============================================================================*/
void task::setFlags( const uint32_t xFlags, const bool value ) noexcept
{
    if ( value ) {
        bits::multipleSet( this->flags, xFlags );
    }
    else {
        bits::multipleClear( this->flags, xFlags );
    }
}
/*============================================================================*/
bool task::getFlag( const uint32_t flag ) const noexcept
{
    const uint32_t xBit = this->flags & flag;
    return ( 0UL != xBit );
}
/*============================================================================*/
priority_t task::getPriority( void ) const noexcept
{
    return priority;
}
/*============================================================================*/
bool task::setPriority( const priority_t pValue ) noexcept
{
    bool retValue = false;

    if ( pValue < static_cast<priority_t>( Q_PRIORITY_LEVELS ) ) {
        priority = pValue;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
cycles_t task::getCycles( void ) const noexcept
{
    return cycles;
}
/*============================================================================*/
taskState task::getState( void ) const noexcept
{
    taskState retValue;

    retValue = static_cast<taskState>( getFlag( BIT_SHUTDOWN ) );
    if ( taskState::DISABLED_STATE != retValue ) { /*Task is awaken*/
        retValue = static_cast<taskState>( getFlag( BIT_ENABLED ) );
    }

    return retValue;
}
/*============================================================================*/
bool task::deadLineReached( void ) const noexcept
{
    bool retValue = false;

    if ( getFlag( BIT_ENABLED ) ) {
        const iteration_t iters = iterations;
        /*task should be periodic or must have available iters*/
        if ( ( TASK_ITER_VALUE( iters ) > 0 ) || ( PERIODIC == iters ) ) {
            /*check the time deadline*/
            const clock_t interval = time.getInterval();
            const bool expired = time.expired();

            if ( ( 0UL == interval ) || expired ) {
                retValue = true;
            }
        }
    }

    return retValue;
}
/*============================================================================*/
bool task::setState( const taskState s ) noexcept
{
    bool retValue = false;

    switch ( s ) {
        case taskState::DISABLED_STATE: case taskState::ENABLED_STATE: // skipcq: CXX-C1001
            if ( s != static_cast<taskState>( getFlag( BIT_ENABLED ) ) ) {
                setFlags( BIT_ENABLED, static_cast<bool>( s ) );
                time.reload();
            }
            retValue = true;
            break;
        case taskState::ASLEEP_STATE:
            setFlags( BIT_SHUTDOWN, false );
            retValue = true;
            break;
        case taskState::AWAKE_STATE:
            setFlags( BIT_SHUTDOWN, true );
            retValue = true;
            break;
        default:
            break;
    }

    return retValue;
}
/*============================================================================*/
void task::setIterations( const iteration_t iValue ) noexcept
{
    if ( iValue > 0 ) {
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
bool task::setTime( const qOS::duration_t tValue ) noexcept
{
    return time.set( tValue );
}
/*============================================================================*/
bool task::setCallback( const taskFcn_t tCallback ) noexcept
{
    bool retValue = false;

    if ( tCallback != callback ) {
        callback = tCallback;
        #if ( ( Q_FSM == 1 ) || ( Q_CLI == 1 ) )
            aObj = nullptr;
        #endif
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool task::setData( void *arg ) noexcept
{
    bool retValue = false;

    if ( arg != taskData ) {
        taskData = arg;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool task::setName( const char *tName ) noexcept
{
    bool retValue = false;
    const size_t nl = util::strlen( tName , sizeof(name) );
    /*cstat -MISRAC++2008-5-14-1*/
    if ( ( nullptr != getContainer() ) && ( nl > 0U ) && ( nl < sizeof(name) ) ) {
        if ( nullptr == os.getTaskByName( tName ) ) {
            (void)util::strcpy( name, tName , sizeof( name ) ); // skipcq: CXX-C1000
            retValue = true;
        }
    }
    /*cstat +MISRAC++2008-5-14-1*/
    return retValue;
}
/*============================================================================*/
const char* task::getName( void ) const noexcept
{
    return name; // skipcq: CXX-C1000
}
/*============================================================================*/
trigger task::queueCheckEvents( void ) noexcept
{
    trigger retValue = trigger::None;

    if ( nullptr != aQueue ) {
        bool fullFlag;
        bool countFlag;
        bool receiverFlag;
        bool emptyFlag;
        size_t qCount; /*current queue count*/

        fullFlag = getFlag( BIT_QUEUE_FULL );
        countFlag = getFlag( BIT_QUEUE_COUNT );
        receiverFlag = getFlag( BIT_QUEUE_RECEIVER );
        emptyFlag = getFlag( BIT_QUEUE_EMPTY );

        qCount = aQueue->count(); /*to avoid side effects*/
        /*check the queue events in the corresponding precedence order*/
        /*cstat -MISRAC++2008-5-14-1*/
        if ( fullFlag && aQueue->isFull() ) { /*isFull() is known to not have side effects*/
            retValue = trigger::byQueueFull;
        }
        else if ( ( countFlag ) && ( qCount >= aQueueCount ) ) {
            retValue = trigger::byQueueCount;
        }
        else if ( receiverFlag && ( qCount > 0U ) ) {
            retValue = trigger::byQueueReceiver;
        }
        else if ( emptyFlag && aQueue->isEmpty() ) {  /*isEmpty() is known to not have side effects*/
            /*qQueue_IsEmpty is known to not have side effects*/
            retValue = trigger::byQueueEmpty;
        }
        else {
            /*this case does not need to be handled*/
        }
        /*cstat +MISRAC++2008-5-14-1*/
    }

    return retValue;
}
/*============================================================================*/
size_t task::getID( void ) const noexcept
{
    return entry;
}
/*============================================================================*/
bool task::attachQueue( queue &q, const queueLinkMode mode, const size_t arg ) noexcept
{
    bool retValue = false;

    if ( q.isInitialized() ) {
        setFlags( static_cast<uint32_t>( mode ) & QUEUE_FLAGS_MASK, 0U != arg );
        if ( queueLinkMode::QUEUE_COUNT == mode ) {
            aQueueCount = arg;
        }
        aQueue = ( arg > 0U ) ? &q : nullptr;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
void * const & task::getAttachedObject( void ) const noexcept
{
    return aObj;
}
/*============================================================================*/
