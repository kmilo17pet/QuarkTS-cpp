#include "include/fsm.hpp"
#include "include/helper.hpp"

using namespace qOS;

const sm::timeoutSpecOption_t stateMachine::OPT_INDEX_MASK = 0x00FFFFFFUL;
const sm::timeoutSpecOption_t sm::TIMEOUT_SET_ENTRY = 0x01000000UL;
const sm::timeoutSpecOption_t sm::TIMEOUT_RST_ENTRY = 0x02000000UL;
const sm::timeoutSpecOption_t sm::TIMEOUT_SET_EXIT = 0x04000000UL;
const sm::timeoutSpecOption_t sm::TIMEOUT_RST_EXIT = 0x08000000UL;
const sm::timeoutSpecOption_t sm::TIMEOUT_KEEP_IF_SET = 0x10000000UL;
const sm::timeoutSpecOption_t sm::TIMEOUT_PERIODIC = 0x20000000UL;

/*============================================================================*/
sm::signalPublisher::signalPublisher()
{
    unsubscribeAll();
}
/*============================================================================*/
void sm::signalPublisher::unsubscribeAll( void ) noexcept
{
    for ( size_t i = 0U ; i < static_cast<size_t>( Q_FSM_PS_SIGNALS_MAX ) ; ++i ) {
        psSignals[ i ] = sm::signalID::SIGNAL_NONE;
        for ( size_t j = 0U; j < static_cast<size_t>( Q_FSM_PS_SUB_PER_SIGNAL_MAX ) ; ++j ) {
            psSubs[ i ][ j ] = nullptr;
        }
    }
}
/*============================================================================*/
bool sm::signalPublisher::subscribeToSignal( stateMachine &m, sm::signalID s ) noexcept
{
    bool retValue = false;

    #if ( Q_FSM_PS_SIGNALS_MAX > 0 ) && ( Q_FSM_PS_SUB_PER_SIGNAL_MAX > 0 )
        if ( s < sm::signalID::MAX_SIGNAL ) {
            sm::psIndex_t r = getSubscriptionStatus( m, s );

            retValue = true;
            switch ( r.status ) {
                case sm::PS_SIGNAL_NOT_FOUND:
                    psSignals[ r.sig_slot ] = s;
                    psSubs[ r.sig_slot ][ r.sub_slot ] = &m;
                    break;
                case sm::PS_SUBSCRIBER_NOT_FOUND:
                    psSubs[ r.sig_slot ][ r.sub_slot ] = &m;
                    break;
                case sm::PS_SUBSCRIBER_FOUND:
                    break;
                default:
                    retValue = false;
                    break;
            }
        }
    #else
        Q_UNUSED( s );
    #endif /*( Q_FSM_PS_SIGNALS_MAX > 0 ) && ( Q_FSM_PS_SUB_PER_SIGNAL_MAX > 0 )*/
    return retValue;
}
/*============================================================================*/
sm::psIndex_t sm::signalPublisher::getSubscriptionStatus( stateMachine &m, sm::signalID s ) const noexcept
{
    sm::psIndex_t idx = { sm::PS_SIGNAL_NOT_FOUND, 0U ,0U };
    size_t i;
    size_t j = 0U;
    const size_t maxSig = static_cast<size_t>( Q_FSM_PS_SIGNALS_MAX );
    const size_t maxSub = static_cast<size_t>( Q_FSM_PS_SUB_PER_SIGNAL_MAX );

    for ( i = 0U ; ( i < maxSig ) && ( sm::signalID::SIGNAL_NONE != psSignals[ i ] ) ; ++i ) {
        if ( s == psSignals[ i ] ) {
            idx.status = sm::PS_SUBSCRIBER_NOT_FOUND;
            for ( j = 0U ; ( j < maxSub ) && ( nullptr != psSubs[ i ][ j ] ) ; ++j ) {
                if ( &m == psSubs[ i ][ j ] ) {
                    idx.status = sm::PS_SUBSCRIBER_FOUND;
                    break;
                }
            }
        }
        if ( sm::PS_SIGNAL_NOT_FOUND != idx.status ) {
            break;
        }
    }
    if ( i >= maxSig ) {
        idx.status = sm::PS_SIGNAL_SLOTS_FULL;
    }
    if ( j >= maxSub ) {
        idx.status = sm::PS_SUBSCRIBER_SLOTS_FULL;
    }

    idx.sig_slot = i;
    idx.sub_slot = j;
    return idx;
}
/*============================================================================*/
bool sm::signalPublisher::unsubscribeFromSignal( stateMachine &m, sm::signalID s ) noexcept
{
    bool retValue = false;
    #if ( Q_FSM_PS_SIGNALS_MAX > 0 ) && ( Q_FSM_PS_SUB_PER_SIGNAL_MAX > 0 )
        if ( s < sm::signalID::MAX_SIGNAL ) {
            sm::psIndex_t r = getSubscriptionStatus( m, s );

            if ( sm::PS_SUBSCRIBER_FOUND == r.status ) {
                size_t i;
                size_t li = static_cast<size_t>( Q_FSM_PS_SUB_PER_SIGNAL_MAX ) - 1U;

                for ( i = r.sub_slot ; i < li ; ++i ) {
                    psSubs[ r.sig_slot ][ i ] = psSubs[ r.sig_slot ][ i + 1U ];
                }
                psSubs[ r.sig_slot ][ li ] = nullptr;

                if ( nullptr == psSubs[ r.sig_slot ][ 0 ] ) { /*no subscribers left?*/
                    /*remove the signal from the psSignals list*/
                    li = static_cast<size_t>( Q_FSM_PS_SIGNALS_MAX ) - 1U;
                    for ( i = r.sig_slot ; i < li ; ++i ) {
                        psSignals[ i ] = psSignals[ i + 1U ];
                    }
                    psSignals[ li ] = sm::signalID::SIGNAL_NONE;
                }
                retValue = true;
            }
        }
    #else
        Q_UNUSED( s );
    #endif /*( Q_FSM_PS_SIGNALS_MAX > 0 ) && ( Q_FSM_PS_SUB_PER_SIGNAL_MAX > 0 )*/
    return retValue;
}
/*============================================================================*/
bool sm::signalPublisher::sendSignal( sm::signalID sig, void *sData, bool isUrgent ) noexcept
{
    uint8_t r = 0U;

    if ( sig < sm::signalID::SIGNAL_NONE ) {
        #if ( Q_FSM_PS_SIGNALS_MAX > 0 )  && ( Q_FSM_PS_SUB_PER_SIGNAL_MAX > 0 )
            const size_t maxSig = static_cast<size_t>( Q_FSM_PS_SIGNALS_MAX );
            const size_t maxSub = static_cast<size_t>( Q_FSM_PS_SUB_PER_SIGNAL_MAX );
            for ( size_t i = 0U ; ( i < maxSig ) && ( sm::signalID::SIGNAL_NONE != psSignals[ i ] ) ; ++i ) {
                if ( sig == psSignals[ i ] ) {
                    r = 1U;
                    for ( size_t j = 0U ; ( j < maxSub ) && ( nullptr != psSubs[ i ][ j ] ) ; ++j ) {
                        r &= psSubs[ i ][ j ]->sendSignal( sig, sData, isUrgent ) ? 
                        static_cast<uint8_t>( 1U ) : static_cast<uint8_t>( 0U );
                    }
                    break;
                }
            }
        #endif
    }
    return static_cast<bool>( r );
}
/*============================================================================*/
bool sm::stateHandler::timeoutSet( const index_t i, const qOS::duration_t t ) noexcept
{
    return thisMachine().timeoutSet( i, t );
}
/*============================================================================*/
bool sm::stateHandler::timeoutStop( const index_t i ) noexcept
{
    return thisMachine().timeoutStop( i );
}
/*============================================================================*/
bool stateMachine::smSetup( sm::stateCallback_t topFcn, sm::state *init, const sm::surroundingCallback_t sFcn, void* pData ) noexcept
{
    current = nullptr;
    next = nullptr;
    source = nullptr;
    mData = pData;
    surrounding = sFcn;
    sm::stateHandler::Machine = this;
    sm::stateHandler::Data = pData;
    top.topSelf( topFcn, init );
    return true;
}
/*============================================================================*/
void sm::state::topSelf( const sm::stateCallback_t topFcn, sm::state *init ) noexcept
{
    lastRunningChild = init;
    initState = init;
    sCallback = topFcn;
    parent = nullptr;
    tTable = nullptr;
    tEntries = 0U;
    tdef = nullptr;
    nTm = 0U;
}
/*============================================================================*/
bool sm::state::subscribe( sm::state *s, const sm::stateCallback_t sFcn, sm::state *init ) noexcept
{
    int retValue = false;

    if ( ( s != this ) && ( init != this ) ) {
        s->lastRunningChild = init;
        s->initState = init;
        s->sCallback = sFcn;
        s->parent = this;
        s->tTable = nullptr;
        s->tEntries = 0U;
        s->tdef = nullptr;
        s->nTm = 0U;
        retValue = true;
    }
    return retValue;
}
/*============================================================================*/
bool sm::state::setTransitions( sm::transition_t *table, size_t n ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != table ) && ( n > 0U ) ) {
        tTable = table;
        tEntries = n;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool stateMachine::installSignalQueue( queue& q ) noexcept
{
    bool retValue = false;
    #if ( Q_QUEUES == 1 )
        /*cstat -MISRAC++2008-5-14-1*/
        if ( ( q.isInitialized() ) && ( sizeof(sm::signal_t) == q.getItemSize() ) ) {
            sQueue = &q; /*install the queue*/
            retValue = true;
        }
        /*cstat +MISRAC++2008-5-14-1*/
        
    #else
        Q_UNUSED( q );
    #endif

    return retValue;
}
/*============================================================================*/
void sm::state::sweepTransitionTable( sm::stateHandler &h ) const noexcept
{
    const size_t n = tEntries;
    const sm::transition_t *iTransition;
    
    for ( size_t i = 0U ; i < n ; ++i ) {
        iTransition = &tTable[ i ]; /*get the i-element from the table*/
        if ( ( h.Signal >= sm::signalID::TM_MIN ) && ( h.Signal <= sm::signalID::TM_MAX ) ) {
            h.SignalData = nullptr; /*ignore signal data on timeout signals*/
        }
        if ( ( h.Signal == iTransition->xSignal ) && ( h.SignalData == iTransition->signalData ) ) { /*table entry match*/
            bool transitionAllowed = true; /*allow the transition by default*/

            if ( nullptr != iTransition->guard ) {
                transitionAllowed = iTransition->guard( h );
            }
            if ( transitionAllowed ) {
                if ( nullptr != iTransition->nextState ) {
                    h.NextState = iTransition->nextState;
                    h.TransitionHistory = iTransition->history;
                    break;
                }
            }
        }
    }
}
/*============================================================================*/
bool stateMachine::internalSignalSend( sm::signalID sig, void *sData, bool isUrgent ) noexcept
{
    bool retValue = false;
    #if ( Q_QUEUES == 1 )
        /*check if the state-machine has a signal queue*/
        if ( nullptr != sQueue ) {
            sm::signal_t sig_msg;
            sig_msg.id = sig;
            sig_msg.data = sData;

            retValue = sQueue->send( &sig_msg, isUrgent ? queueSendMode::TO_FRONT : queueSendMode::TO_BACK );
        }
    #else
        Q_UNUSED( isUrgent );
    #endif
    /*cppcheck-suppress knownConditionTrueFalse */
    if ( ( !retValue ) && ( sm::signalID::SIGNAL_NONE == signalNot.id ) ) {
        signalNot.id = sig;
        signalNot.data = sData;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool stateMachine::sendSignal( sm::signalID sig, void *sData, bool isUrgent ) noexcept
{
    bool retValue = false;

    if ( sig < sm::signalID::SIGNAL_NONE ) {
        retValue = internalSignalSend( sig, sData, isUrgent );
    }

    return retValue;
}
/*============================================================================*/
void stateMachine::timeoutCheckSignals( void ) noexcept
{
    for ( size_t i = 0U ; i < static_cast<size_t>( Q_FSM_MAX_TIMEOUTS ) ; ++i ) {
        if ( timeSpec->timeout[ i ].expired() ) {
            (void)sendSignal( sm::SIGNAL_TIMEOUT( i ), nullptr, false );
            if ( 0UL  != ( timeSpec->isPeriodic & ( 1UL <<  i ) ) ) {
                timeSpec->timeout[ i ].reload();
            }
            else {
                timeSpec->timeout[ i ].disarm();
            }
        }
    }
}
/*============================================================================*/
void stateMachine::timeoutPerformSpecifiedActions( sm::state * const s, sm::signalID sig ) noexcept
{
    sm::timeoutStateDefinition_t * const tbl = s->tdef;
    const size_t n = s->nTm;

    if ( ( n > 0U ) && ( nullptr != tbl ) ) {
        sm::timeoutSpecOption_t setCheck;
        sm::timeoutSpecOption_t resetCheck;

        if ( sm::signalID::SIGNAL_ENTRY == sig ) {
            setCheck = sm::TIMEOUT_SET_ENTRY;
            resetCheck = sm::TIMEOUT_RST_ENTRY;
        }
        else {
            setCheck = sm::TIMEOUT_SET_EXIT;
            resetCheck = sm::TIMEOUT_RST_EXIT;
        }
        for ( size_t i = 0U ; i < n ; ++i ) { /*loop table */
            const sm::timeoutSpecOption_t opt = tbl[ i ].options;
            const index_t index = static_cast<index_t>( opt & OPT_INDEX_MASK );
            /*state match and index is valid?*/
            if ( index < static_cast<index_t>( Q_FSM_MAX_TIMEOUTS ) ) {
                timer * const tmr = &timeSpec->timeout[ index ];
                const qOS::duration_t tValue = tbl[ i ].xTimeout;

                if ( 0UL != ( opt & setCheck ) ) {
                    if ( 0UL == ( opt & sm::TIMEOUT_KEEP_IF_SET ) ) {
                        (void)tmr->set( tValue );
                    }
                    if ( 0UL != ( opt & sm::TIMEOUT_PERIODIC ) ) {
                        bits::singleSet( timeSpec->isPeriodic, index );
                    }
                    else {
                        bits::singleClear( timeSpec->isPeriodic, index );
                    }
                }
                if ( 0UL != ( opt & resetCheck ) ) {
                    tmr->disarm();
                }
            }
        }
    }
}
/*============================================================================*/
bool stateMachine::installTimeoutSpec( sm::timeoutSpec &ts ) noexcept
{
    for ( size_t i = 0U ; i < static_cast<size_t>( Q_FSM_MAX_TIMEOUTS ) ; ++i ) {
        ts.timeout[ i ].disarm();
        ts.isPeriodic = 0U;
    }
    timeSpec = &ts;

    return true;
}
/*============================================================================*/
bool sm::state::setTimeouts( sm::timeoutStateDefinition_t *def, size_t n ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != tdef ) && ( n > 0U ) ) {
        tdef = def;
        nTm = n;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool stateMachine::timeoutSet( const index_t xTimeout, const qOS::duration_t t ) noexcept
{
    bool retValue = false;

    if ( xTimeout < static_cast<index_t>( Q_FSM_MAX_TIMEOUTS ) ) {
        if ( nullptr != timeSpec ) {
            retValue = timeSpec->timeout[ xTimeout ].set( t );
        }
    }

    return retValue;
}
/*============================================================================*/
bool stateMachine::timeoutStop( const index_t xTimeout ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != timeSpec ) && ( xTimeout < static_cast<index_t>( Q_FSM_MAX_TIMEOUTS ) ) ) {
        #if ( Q_QUEUES == 1 )
            size_t cnt;
            sm::signal_t xSignal;

            cnt = sQueue->count();
            while ( 0U != cnt-- ) {
                if ( sQueue->receive( &xSignal ) ) {
                    if ( xSignal.id != sm::SIGNAL_TIMEOUT( xTimeout ) ) {
                        /*keep non-timeout signals*/
                        (void)sQueue->send( &xSignal, queueSendMode::TO_BACK );
                    }
                }
            }
        #endif
        timeSpec->timeout[ xTimeout ].disarm();
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
const sm::state& stateMachine::getTop( void ) const noexcept
{
    return top;
}
/*============================================================================*/
sm::state * const & stateMachine::getCurrent( void ) const noexcept
{
    return current;
}
/*============================================================================*/
queue * const & stateMachine::getQueue( void ) const noexcept
{
    return sQueue;
}
/*============================================================================*/
sm::timeoutSpec * const & stateMachine::getTimeSpec( void ) const noexcept
{
    return timeSpec;
}
/*============================================================================*/
void * const & stateMachine::getData( void ) const noexcept
{
    return mData;
}
/*============================================================================*/
void* sm::state::getData( void ) noexcept
{
    return sData;
}
/*============================================================================*/
void sm::state::setData( void* pData ) noexcept
{
    sData = pData;
}
/*============================================================================*/
sm::transition_t* sm::state::getTransitionTable( void ) noexcept
{
    return tTable;
}
/*============================================================================*/
void sm::state::setCallback( const sm::stateCallback_t sFcn ) noexcept
{
    sCallback = sFcn;
}
/*============================================================================*/
void stateMachine::setSurrounding( const sm::surroundingCallback_t sFcn ) noexcept
{
    surrounding = sFcn;
}
/*============================================================================*/
void stateMachine::transition( sm::state *target, sm::historyMode mHistory ) noexcept
{
    exitUpToLCA( levelsToLCA( target ) );
    /*then, handle the required history mode*/
    if ( sm::historyMode::NO_HISTORY == mHistory ) {
        /*just restore the default transition*/
        target->lastRunningChild = target->initState;
    }
    else if ( sm::historyMode::SHALLOW_HISTORY == mHistory ) {
        if ( nullptr != target->lastRunningChild ) {
            /*cstat -MISRAC++2008-7-1-1*/
            sm::state *lrc = target->lastRunningChild;
            /*cstat -MISRAC++2008-7-1-1*/
            /*restore the default transition in the last running nested state*/
            lrc->lastRunningChild = lrc->initState;
        }
    }
    else {
        /*
        nothing to do : qSM_TRANSITION_DEEP_HISTORY handled by default all deep
        history its preserved in the "lastRunningChild" attribute, so there is
        no need to change it. qStateMachine_StateOnStart will assign this by
        default.
        */
    }
    next = target; /*notify SM that there was indeed a transition*/
}
/*============================================================================*/
uint8_t stateMachine::levelsToLCA( sm::state *target ) const noexcept
{
    uint8_t xLca = 0U;

    /*
    To discover which exit actions to execute, it is necessary to find the
    LCA(LeastCommon Ancestor) of the source and target states.
    */
    if ( source == target ) {
        xLca = 1U; /*recursive transition, only a level needs to be performed*/
    }
    else {
        bool xBreak = false;
        uint8_t n = 0U;

        for ( sm::state *s = source ; ( nullptr != s ) && ( !xBreak ) ; s = s->parent ) {
            /*cstat -MISRAC++2008-6-5-2*/
            for ( sm::state *t = target ; nullptr != t ; t = t->parent ) {
                if ( s == t ) {
                    xLca = n;
                    xBreak = true;
                    break;
                }
            }
            /*cstat +MISRAC++2008-6-5-2*/
            ++n;
        }
    }

    return xLca; /*return # of levels from the current state to the LCA.*/
}
/*============================================================================*/
void stateMachine::exitUpToLCA( uint8_t lca ) noexcept
{
    sm::state *s = current;

    /*exit current states*/
    while ( s != source ) {
        s = stateOnExit( s );
    }
    /*exit all superstates up to LCA*/
    while ( 0U != lca-- ) {
        s = stateOnExit( s );
    }
    current = s;
}
/*============================================================================*/
sm::status sm::state::activities( void )
{
    /*cstat -MISRAC++2008-5-0-3*/
    return ( nullptr != sCallback ) ? sCallback( *pHandler ) : sm::status::FAILURE;
    /*cstat +MISRAC++2008-5-0-3*/
}
/*============================================================================*/
sm::status stateMachine::invokeStateActivities( sm::state * const s ) noexcept
{
    /*cstat -CERT-EXP39-C_d*/
    sm::stateHandler * const pHandler = static_cast<sm::stateHandler*>( this );
    s->pHandler = pHandler;
    /*cstat CERT-EXP39-C_d*/
    if ( nullptr != surrounding ) {
        sm::stateHandler::Status = sm::status::BEFORE_ANY;
        surrounding( *pHandler );
    }

    sm::stateHandler::Status = sm::status::ABSENT;
    sm::stateHandler::Status = s->activities();

    if ( nullptr != surrounding ) {
        if ( sm::stateHandler::Status < sm::status::FAILURE ) {
            sm::stateHandler::Status = sm::status::FAILURE;
        }
        surrounding( *pHandler );
    }

    return sm::stateHandler::Status;
}
/*============================================================================*/
void stateMachine::prepareHandler( sm::signal_t sig, sm::state *s ) noexcept
{
    sm::stateHandler::Signal = sig.id;
    sm::stateHandler::SignalData = sig.data;
    sm::stateHandler::NextState = nullptr;
    sm::stateHandler::StartState = nullptr;
    sm::stateHandler::Status = sm::status::ABSENT;
    sm::stateHandler::TransitionHistory = sm::historyMode::NO_HISTORY;
    sm::stateHandler::StateData = s->sData;
    sm::stateHandler::Data = mData;
    sm::stateHandler::State = s;
}
/*============================================================================*/
sm::state* stateMachine::stateOnExit( sm::state *s ) noexcept
{
    sm::signal_t SIG_MSG_EXIT;
    SIG_MSG_EXIT.id = sm::signalID::SIGNAL_EXIT;

    prepareHandler( SIG_MSG_EXIT, s );
    (void)invokeStateActivities( s );
    if ( ( nullptr != timeSpec ) && ( nullptr != s->tdef ) ) {
        timeoutPerformSpecifiedActions( s, sm::signalID::SIGNAL_EXIT );
    }
    s->parent->lastRunningChild = s;

    return s->parent;
}
/*============================================================================*/
void stateMachine::stateOnEntry( sm::state *s ) noexcept
{
    sm::signal_t SIG_MSG_ENTRY;
    SIG_MSG_ENTRY.id = sm::signalID::SIGNAL_ENTRY;

    prepareHandler( SIG_MSG_ENTRY, s );
    (void)invokeStateActivities( s );

    if ( ( nullptr != timeSpec ) && ( nullptr != s->tdef ) ) {
        timeoutPerformSpecifiedActions( s, sm::signalID::SIGNAL_ENTRY );
    }
}
/*============================================================================*/
sm::state* stateMachine::stateOnStart( sm::state *s ) noexcept
{
    sm::signal_t SIG_MSG_START;
    SIG_MSG_START.id = sm::signalID::SIGNAL_START;

    prepareHandler( SIG_MSG_START, s );
    (void)invokeStateActivities( s );

    if ( nullptr != sm::stateHandler::StartState ) {
        /*changes from callback takes more precedence*/
        next = sm::stateHandler::StartState;
    }
    else {
        if ( nullptr != s->lastRunningChild ) {
            /*try to preserve history*/
            next = s->lastRunningChild;
        }
    }

    return next;
}
/*============================================================================*/
sm::status stateMachine::stateOnSignal( sm::state *s, sm::signal_t sig ) noexcept
{
    sm::status status;

    prepareHandler( sig, s );
    if ( nullptr != s->tTable ) {
        /*evaluate the transition table if available*/
        /*cstat -CERT-EXP39-C_d*/
        s->sweepTransitionTable( *static_cast<sm::stateHandler*>( this ) );
        /*cstat +CERT-EXP39-C_d*/
    }
    status = invokeStateActivities( s );

    if ( nullptr != sm::stateHandler::NextState ) { /*perform the transition if available*/
        transition( sm::stateHandler::NextState, sm::stateHandler::TransitionHistory );
        /*the signal is assumed to be handled if the transition occurs*/
        status = sm::status::SIGNAL_HANDLED;
    }

    return status;
}
/*============================================================================*/
void stateMachine::tracePathAndRetraceEntry( sm::state **trace ) noexcept
{
    sm::state *s;
    /*
    Entry actions must be executed in order form the least deeply nested to the
    most deeply nested state. This is opposite to the normal navigability of the
    module data-structure design. So this is solved by first recording the entry
    path from the LCA to the target, then playing it backwards. with execution
    of entry actions.
    qStateMachine_TracePathAndRetraceEntry and qStateMachine_TraceOnStart are
    used to handle this after the transition perform all the required exit
    actions.
    */
    *trace = nullptr;
    /*cstat -MISRAC++2008-6-5-4 -MISRAC++2008-6-5-2*/
    for ( s = next ; s != current ; s = s->parent ) {
        *(++trace) = s; /* trace path to target */
    }
    /*cstat -MISRAC++2008-6-2-1 +-MISRAC++2008-6-5-4 +MISRAC++2008-6-5-2*/
    while ( nullptr != ( s = *trace-- ) ) { /* retrace entry from LCA */
        stateOnEntry( s );
    }
    /*cstat +MISRAC++2008-6-2-1*/
    current = next;
    next = nullptr;
}
/*============================================================================*/
void stateMachine::traceOnStart( sm::state **entryPath ) noexcept
{
    while ( nullptr != stateOnStart( current ) ) {
        tracePathAndRetraceEntry( entryPath );
    }
}
/*============================================================================*/
sm::signal_t stateMachine::checkForSignals( sm::signal_t sig ) noexcept
{
    sm::signal_t xSignal = sig;

    /*
    initially checks for time signals if the state machine has a
    timeout-specification object installed. If any timeout expires, the signal
    is put on the available recipient.
    */
    if ( nullptr != timeSpec ) {
        timeoutCheckSignals();
        /*use the available recipient: signal-queue or the EV*/
    }
    /*
    If the state machine has the signal-queue installed, the signal that is in
    front of the queue is obtained. In case of there is no signal-queue or the
    queue its empty, the input argument and the exclusion variable are verified.
    */
    #if ( Q_QUEUES == 1 )
        if ( nullptr != sQueue ) {
            sm::signal_t receivedSignal;

            if ( sQueue->receive( &receivedSignal ) ) {
                xSignal = receivedSignal;
            }
        }
    #endif
    /*A signal coming from the signal-queue has the higher precedence.*/
    if ( ( sm::signalID::SIGNAL_NONE == xSignal.id ) && ( sm::signalID::SIGNAL_NONE != signalNot.id ) ) {
        xSignal = signalNot; /*exclusion variable*/
        signalNot.id = sm::signalID::SIGNAL_NONE;
        signalNot.data = nullptr;
    }

    return xSignal;
}
/*============================================================================*/
bool stateMachine::run( sm::signal_t sig ) noexcept
{
    bool retValue = false;
    sm::state *entryPath[ Q_FSM_MAX_NEST_DEPTH ]; // skipcq: CXX-W2066

    sig = checkForSignals( sig );
    if ( nullptr == current ) { /*Enter here only once to start the top state*/
        current = &top;
        next = nullptr;
        stateOnEntry( current );
        traceOnStart( entryPath ); // skipcq: CXX-C1000
    }
    /*evaluate the hierarchy until the signal is handled*/
    /*cstat -MISRAC++2008-6-5-2*/
    for ( sm::state *s = current ; nullptr != s ; s = s->parent ) {
        source = s; /* level of outermost event handler */
        if ( sm::status::SIGNAL_HANDLED == stateOnSignal( s, sig ) ) {
            if ( nullptr != next ) {  /* state transition taken? */
                /*run entry/start actions in the rest of the hierarchy after transition*/
                tracePathAndRetraceEntry( entryPath ); // skipcq: CXX-C1000
                traceOnStart( entryPath ); // skipcq: CXX-C1000
            }
            retValue = true;
            break;/* signal processed */
        }
    }
    /*cstat +MISRAC++2008-6-5-2*/
    return retValue;
}
/*============================================================================*/
