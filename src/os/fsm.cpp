#include "fsm.hpp"
#include "helper.hpp"

using namespace qOS;

/*cstat -MISRAC++2008-8-5-2*/
static fsm::signalID psSignals[ Q_FSM_PS_SIGNALS_MAX ] = { fsm::signalID::SIGNAL_START };
/*cstat +MISRAC++2008-8-5-2*/
static stateMachine *psSubs[ Q_FSM_PS_SIGNALS_MAX ][ Q_FSM_PS_SUB_PER_SIGNAL_MAX ];

static const fsm::timeoutSpecOption_t OPT_INDEX_MASK = 0x00FFFFFFuL;

const fsm::timeoutSpecOption_t fsm::TIMEOUT_SET_ENTRY = 0x01000000uL;
const fsm::timeoutSpecOption_t fsm::TIMEOUT_RST_ENTRY = 0x02000000uL;
const fsm::timeoutSpecOption_t fsm::TIMEOUT_SET_EXIT = 0x04000000uL;
const fsm::timeoutSpecOption_t fsm::TIMEOUT_RST_EXIT = 0x08000000uL;
const fsm::timeoutSpecOption_t fsm::TIMEOUT_KEEP_IF_SET = 0x10000000uL;
const fsm::timeoutSpecOption_t fsm::TIMEOUT_PERIODIC = 0x20000000uL;

/*============================================================================*/
bool fsm::_Handler::timeoutSet( index_t i, qOS::time_t t ) noexcept
{
    return thisMachine()->timeoutSet( i, t );
}
/*============================================================================*/
bool fsm::_Handler::timeoutStop( index_t i ) noexcept
{
    return thisMachine()->timeoutStop( i );
}
/*============================================================================*/
bool stateMachine::setup( fsm::stateCallback_t topFcn, fsm::state *init, fsm::surroundingCallback_t sFcn, void* pData ) noexcept
{
    current = nullptr;
    next = nullptr;
    source = nullptr;
    mData = pData;
    surrounding = sFcn;
    fsm::_Handler::Machine = this;
    fsm::_Handler::Data = pData;
    top.topSelf( topFcn, init );

    #if ( Q_FSM_PS_SIGNALS_MAX > 0 )  && ( Q_FSM_PS_SUB_PER_SIGNAL_MAX > 0 )
    if ( psSignals[ 0 ] > fsm::signalID::SIGNAL_NONE ) {
        unsubscribeAll();
    }
    #endif
    return true;
}
/*============================================================================*/
void fsm::state::topSelf( fsm::stateCallback_t topFcn, fsm::state *init ) noexcept
{
    lastRunningChild = init;
    initState = init;
    sCallback = topFcn;
    parent = nullptr;
    tTable = nullptr;
    tEntries = 0u;
    tdef = nullptr;
    nTm = 0u;
}
/*============================================================================*/
bool fsm::state::subscribe( fsm::state *s, fsm::stateCallback_t sFcn, fsm::state *init ) noexcept
{
    s->lastRunningChild = init;
    s->initState = init;
    s->sCallback = sFcn;
    s->parent = this;
    s->tTable = nullptr;
    s->tEntries = 0u;
    s->tdef = nullptr;
    s->nTm = 0u;

    return true;
}
/*============================================================================*/
void stateMachine::unsubscribeAll( void ) noexcept
{
    for ( std::size_t i = 0u ; i < static_cast<std::size_t>( Q_FSM_PS_SIGNALS_MAX ) ; ++i ) {
        psSignals[ i ] = fsm::signalID::SIGNAL_NONE;
        for ( std::size_t j = 0u; j < static_cast<std::size_t>( Q_FSM_PS_SUB_PER_SIGNAL_MAX ) ; ++j ) {
            psSubs[ i ][ j ] = nullptr;
        }
    }
}
/*============================================================================*/
bool fsm::state::setTransitions( fsm::transition_t *table, std::size_t n ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != table ) && ( n > 0u ) ) {
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
        if ( ( true == q.isReady() ) && ( sizeof(fsm::signal_t) == q.getItemSize() ) ) {
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
void fsm::state::sweepTransitionTable( fsm::_Handler &h ) noexcept
{
    std::size_t n;
    fsm::transition_t *iTransition;
    n = tEntries;
    
    for ( std::size_t i = 0u ; i < n ; ++i ) {
        bool transitionAllowed = true; /*allow the transition by default*/

        iTransition = &tTable[ i ]; /*get the i-element from the table*/
        if ( ( h.Signal >= fsm::signalID::TM_MIN ) && ( h.Signal <= fsm::signalID::TM_MAX ) ) {
            h.SignalData = nullptr; /*ignore signal data on timeout signals*/
        }
        if ( ( h.Signal == iTransition->xSignal ) && ( h.SignalData == iTransition->signalData ) ) { /*table entry match*/
            if ( nullptr != iTransition->guard ) {
                /*if signal-guard available, run the guard function*/
                transitionAllowed = iTransition->guard( h ); /*cast allowed, struct layout compatible*/
            }
            if ( transitionAllowed ) {
                if ( nullptr != iTransition->nextState ) {
                    /*if target state available, set the transition from table*/
                    h.NextState = iTransition->nextState;
                    /*set the history mode form the table*/
                    h.TransitionHistory = iTransition->history;
                    break;
                }
            }
        }
    }
}
/*============================================================================*/
bool stateMachine::internalSignalSend( fsm::signalID sig, void *sData, bool isUrgent ) noexcept
{
    bool retValue = false;
    #if ( Q_QUEUES == 1 )
        /*check if the state-machine has a signal queue*/
        if ( nullptr != sQueue ) {
            fsm::signal_t sig_msg = { sig, sData };

            retValue = sQueue->send( &sig_msg, isUrgent ? queueSendMode::TO_FRONT : queueSendMode::TO_BACK );
        }
    #else
        Q_UNUSED( isUrgent );
    #endif

    if ( ( false == retValue ) && ( fsm::signalID::SIGNAL_NONE == signalNot.id ) ) {
        signalNot.id = sig;
        signalNot.data = sData;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool stateMachine::sendSignal( fsm::signalID sig, void *sData, bool isUrgent ) noexcept
{
    bool retValue = false;

    if ( sig < fsm::signalID::SIGNAL_NONE ) {
        retValue = internalSignalSend( sig, sData, isUrgent );
    }

    return retValue;
}
/*============================================================================*/
bool stateMachine::sendSignalToSubscribers( fsm::signalID sig, void *sData, bool isUrgent ) noexcept
{
    std::uint8_t r = 0u;

    if ( sig < fsm::signalID::SIGNAL_NONE ) {
        #if ( Q_FSM_PS_SIGNALS_MAX > 0 )  && ( Q_FSM_PS_SUB_PER_SIGNAL_MAX > 0 )
            const std::size_t maxSig = static_cast<std::size_t>( Q_FSM_PS_SIGNALS_MAX );
            const std::size_t maxSub = static_cast<std::size_t>( Q_FSM_PS_SUB_PER_SIGNAL_MAX );
            for ( std::size_t i = 0u ; ( i < maxSig ) && ( fsm::signalID::SIGNAL_NONE != psSignals[ i ] ) ; ++i ) {
                if ( sig == psSignals[ i ] ) {
                    r = 1u;
                    for ( std::size_t j = 0u ; ( j < maxSub ) && ( nullptr != psSubs[ i ][ j ] ) ; ++j ) {
                        r &= psSubs[ i ][ j ]->sendSignal( sig, sData, isUrgent ) ? 1u : 0u;
                    }
                    break;
                }
            }
        #endif
    }

    return ( 0u != r )? true : false;
}
/*============================================================================*/
void stateMachine::timeoutCheckSignals( void ) noexcept
{
    for ( std::size_t i = 0u ; i < static_cast<std::size_t>( Q_FSM_MAX_TIMEOUTS ) ; ++i ) {
        if ( timeSpec->timeout[ i ].expired() ) {
            (void)sendSignal( fsm::SIGNAL_TIMEOUT( i ), nullptr, false );
            if ( 0uL  != ( timeSpec->isPeriodic & ( 1uL <<  i ) ) ) {
                timeSpec->timeout[ i ].reload();
            }
            else {
                timeSpec->timeout[ i ].disarm();
            }
        }
    }
}
/*============================================================================*/
void stateMachine::timeoutPerformSpecifiedActions( fsm::state * const s, fsm::signalID sig ) noexcept
{
    fsm::timeoutStateDefinition_t * const tbl = s->tdef;
    const std::size_t n = s->nTm;

    if ( ( n > 0u ) && ( nullptr != tbl ) ) {
        fsm::timeoutSpecOption_t setCheck;
        fsm::timeoutSpecOption_t resetCheck;

        if ( fsm::signalID::SIGNAL_ENTRY == sig ) {
            setCheck = fsm::TIMEOUT_SET_ENTRY;
            resetCheck = fsm::TIMEOUT_RST_ENTRY;
        }
        else {
            setCheck = fsm::TIMEOUT_SET_EXIT;
            resetCheck = fsm::TIMEOUT_RST_EXIT;
        }
        for ( std::size_t i = 0u ; i < n ; ++i ) { /*loop table */
            const fsm::timeoutSpecOption_t opt = tbl[ i ].options;
            const index_t index = static_cast<index_t>( opt & OPT_INDEX_MASK );
            /*state match and index is valid?*/
            if ( index < static_cast<index_t>( Q_FSM_MAX_TIMEOUTS ) ) {
                timer * const tmr = &timeSpec->timeout[ index ];
                const qOS::time_t tValue = tbl[ i ].xTimeout;

                if ( 0uL != ( opt & setCheck ) ) {
                    if ( 0uL == ( opt & fsm::TIMEOUT_KEEP_IF_SET ) ) {
                        (void)tmr->set( tValue );
                    }
                    if ( 0uL != ( opt & fsm::TIMEOUT_PERIODIC ) ) {
                        bitSet( timeSpec->isPeriodic, index );
                    }
                    else {
                        bitClear( timeSpec->isPeriodic, index );
                    }
                }
                if ( 0uL != ( opt & resetCheck ) ) {
                    tmr->disarm();
                }
            }
        }
    }
}
/*============================================================================*/
bool stateMachine::installTimeoutSpec( fsm::timeoutSpec &ts ) noexcept
{
    timeSpec = &ts;
    for ( std::size_t i = 0u ; i < static_cast<std::size_t>( Q_FSM_MAX_TIMEOUTS ) ; ++i ) {
        timeSpec->timeout[ i ].disarm();
        timeSpec->isPeriodic = 0u;
    }

    return true;
}
/*============================================================================*/
bool fsm::state::setTimeouts( fsm::timeoutStateDefinition_t *def, std::size_t n ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != tdef ) && ( n > 0u ) ) {
        tdef = def;
        nTm = n;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool stateMachine::timeoutSet( index_t xTimeout, qOS::time_t t ) noexcept
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
bool stateMachine::timeoutStop( index_t xTimeout ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != timeSpec ) && ( xTimeout < static_cast<index_t>( Q_FSM_MAX_TIMEOUTS ) ) ) {
        #if ( Q_QUEUES == 1 )
            std::size_t cnt;
            fsm::signal_t xSignal;

            cnt = sQueue->count();
            while ( 0u != cnt-- ) {
                if ( true == sQueue->receive( &xSignal ) ) {
                    if ( xSignal.id != fsm::SIGNAL_TIMEOUT( xTimeout ) ) {
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
const fsm::state* stateMachine::getTop( void ) const noexcept
{
    return &top;
}
/*============================================================================*/
fsm::state* stateMachine::getCurrent( void ) noexcept
{
    return current;
}
/*============================================================================*/
queue* stateMachine::getQueue( void ) noexcept
{
    return sQueue;
}
/*============================================================================*/
fsm::timeoutSpec* stateMachine::getTimeSpec( void ) noexcept
{
    return timeSpec;
}
/*============================================================================*/
void* stateMachine::getData( void ) noexcept
{
    return mData;
}
/*============================================================================*/
fsm::state* fsm::state::getInit( void ) noexcept
{
    return initState;
}
/*============================================================================*/
fsm::state* fsm::state::getLastState( void ) noexcept
{   
    return lastRunningChild;
}
/*============================================================================*/
fsm::state* fsm::state::getParent( void ) noexcept
{
    return parent;
}
/*============================================================================*/
void* fsm::state::getData( void ) noexcept
{
    return sData;
}
/*============================================================================*/
void fsm::state::setData( void* pData ) noexcept
{
    sData = pData;
}
/*============================================================================*/
fsm::transition_t* fsm::state::getTransitionTable( void ) noexcept
{
    return tTable;
}
/*============================================================================*/
void fsm::state::setCallback( fsm::stateCallback_t sFcn ) noexcept
{
    sCallback = sFcn;
}
/*============================================================================*/
void stateMachine::setSurrounding( fsm::surroundingCallback_t sFcn ) noexcept
{
    surrounding = sFcn;
}
/*============================================================================*/
fsm::psIndex_t stateMachine::getSubscriptionStatus( fsm::signalID s ) noexcept
{
    fsm::psIndex_t idx = { fsm::PS_SIGNAL_NOT_FOUND, 0u ,0u };
    std::size_t i, j = 0u;
    const std::size_t maxSig = static_cast<std::size_t>( Q_FSM_PS_SIGNALS_MAX );
    const std::size_t maxSub = static_cast<std::size_t>( Q_FSM_PS_SUB_PER_SIGNAL_MAX );

    for ( i = 0u ; ( i < maxSig ) && ( fsm::signalID::SIGNAL_NONE != psSignals[ i ] ) ; ++i ) {
        if ( s == psSignals[ i ] ) {
            idx.status = fsm::PS_SUBSCRIBER_NOT_FOUND;
            for ( j = 0u ; ( j < maxSub ) && ( nullptr != psSubs[ i ][ j ] ) ; ++j ) {
                if ( this == psSubs[ i ][ j ] ) {
                    idx.status = fsm::PS_SUBSCRIBER_FOUND;
                    break;
                }
            }
        }
        if ( fsm::PS_SIGNAL_NOT_FOUND != idx.status ) {
            break;
        }
    }
    if ( i >= maxSig ) {
        idx.status = fsm::PS_SIGNAL_SLOTS_FULL;
    }
    if ( j >= maxSub ) {
        idx.status = fsm::PS_SUBSCRIBER_SLOTS_FULL;
    }

    idx.sig_slot = i;
    idx.sub_slot = j;
    return idx;
}
/*============================================================================*/
bool stateMachine::subscribeToSignal( fsm::signalID s ) noexcept
{
    bool retValue = false;

    #if ( Q_FSM_PS_SIGNALS_MAX > 0 ) && ( Q_FSM_PS_SUB_PER_SIGNAL_MAX > 0 )
        if ( s < fsm::signalID::MAX_SIGNAL ) {
            fsm::psIndex_t r = getSubscriptionStatus( s );

            retValue = true;
            switch ( r.status ) {
                case fsm::PS_SIGNAL_NOT_FOUND:
                    psSignals[ r.sig_slot ] = s;
                    psSubs[ r.sig_slot ][ r.sub_slot ] = this;
                    break;
                case fsm::PS_SUBSCRIBER_NOT_FOUND:
                    psSubs[ r.sig_slot ][ r.sub_slot ] = this;
                    break;
                case fsm::PS_SUBSCRIBER_FOUND:
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
bool stateMachine::unsubscribeFromSignal( fsm::signalID s ) noexcept
{
    bool retValue = false;
    #if ( Q_FSM_PS_SIGNALS_MAX > 0 ) && ( Q_FSM_PS_SUB_PER_SIGNAL_MAX > 0 )
        if ( s < fsm::signalID::MAX_SIGNAL ) {
            fsm::psIndex_t r = getSubscriptionStatus( s );

            if ( fsm::PS_SUBSCRIBER_FOUND == r.status ) {
                std::size_t i;
                std::size_t li = static_cast<std::size_t>( Q_FSM_PS_SUB_PER_SIGNAL_MAX ) - 1u;

                for ( i = r.sub_slot ; i < li ; ++i ) {
                    psSubs[ r.sig_slot ][ i ] = psSubs[ r.sig_slot ][ i + 1u ];
                }
                psSubs[ r.sig_slot ][ li ] = nullptr;

                if ( nullptr == psSubs[ r.sig_slot ][ 0 ] ) { /*no subscribers left?*/
                    /*remove the signal from the psSignals list*/
                    li = static_cast<std::size_t>( Q_FSM_PS_SIGNALS_MAX ) - 1u;
                    for ( i = r.sig_slot ; i < li ; ++i ) {
                        psSignals[ i ] = psSignals[ i + 1u ];
                    }
                    psSignals[ li ] = fsm::signalID::SIGNAL_NONE;
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
void stateMachine::transition( fsm::state *target, fsm::historyMode mHistory ) noexcept
{
    exitUpToLCA( levelsToLCA( target ) );
    /*then, handle the required history mode*/
    if ( fsm::historyMode::NO_HISTORY == mHistory ) {
        /*just restore the default transition*/
        target->lastRunningChild = target->initState;
    }
    else if ( fsm::historyMode::SHALLOW_HISTORY == mHistory ) {
        if ( nullptr != target->lastRunningChild ) {
            fsm::state *lrc = target->lastRunningChild;
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
uint8_t stateMachine::levelsToLCA( fsm::state *target ) noexcept
{
    std::uint8_t xLca = 0u;

    /*
    To discover which exit actions to execute, it is necessary to find the
    LCA(LeastCommon Ancestor) of the source and target states.
    */
    if ( source == target ) {
        xLca = 1u; /*recursive transition, only a level needs to be performed*/
    }
    else {
        fsm::state *s, *t;
        bool xBreak = false;
        std::uint8_t n = 0u;

        for ( s = source ; ( nullptr != s ) && ( false == xBreak ) ; s = s->parent ) {
            /*cstat -MISRAC++2008-6-5-2*/
            for ( t = target ; nullptr != t ; t = t->parent ) {
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
void stateMachine::exitUpToLCA( std::uint8_t lca ) noexcept
{
    fsm::state *s = current;

    /*exit current states*/
    while ( s != source ) {
        s = stateOnExit( s );
    }
    /*exit all superstates up to LCA*/
    while ( 0u != lca-- ) {
        s = stateOnExit( s );
    }
    current = s;
}
/*============================================================================*/
fsm::status stateMachine::invokeStateCallback( fsm::state * const s ) noexcept
{
    /*cstat -CERT-EXP39-C_d*/
    fsm::_Handler * const pHandler = static_cast<fsm::_Handler*>( this );
    /*cstat CERT-EXP39-C_d*/
    if ( nullptr != surrounding ) {
        fsm::_Handler::Status = fsm::status::BEFORE_ANY;
        surrounding( *pHandler );
    }
    if ( nullptr != s->sCallback ) {
        fsm::_Handler::Status = fsm::status::ABSENT;
        /*execute the state callback (Transitions here are not allowed)*/
        fsm::_Handler::Status = s->sCallback( *pHandler ); /*cast allowed, struct layout its compatible*/
    }
    else {
        fsm::_Handler::Status = fsm::status::FAILURE;
    }
    /*this property can change from the callback, so check it again*/
    if ( nullptr != surrounding ) {
        if ( fsm::_Handler::Status < fsm::status::FAILURE ) {
            fsm::_Handler::Status = fsm::status::FAILURE;
        }
        surrounding( *pHandler ); /*cast allowed, struct layout compatible*/
    }

    return fsm::_Handler::Status;
}
/*============================================================================*/
void stateMachine::prepareHandler( fsm::signal_t sig, fsm::state *s ) noexcept
{
    fsm::_Handler::Signal = sig.id;
    fsm::_Handler::SignalData = sig.data;
    fsm::_Handler::NextState = nullptr;
    fsm::_Handler::StartState = nullptr;
    fsm::_Handler::Status = fsm::status::ABSENT;
    fsm::_Handler::TransitionHistory = fsm::historyMode::NO_HISTORY;
    fsm::_Handler::StateData = s->sData;
    fsm::_Handler::Data = mData;
    fsm::_Handler::State = s;
}
/*============================================================================*/
fsm::state* stateMachine::stateOnExit( fsm::state *s ) noexcept
{
    const fsm::signal_t SIG_MSG_EXIT = { fsm::signalID::SIGNAL_EXIT, nullptr };

    prepareHandler( SIG_MSG_EXIT, s );
    (void)invokeStateCallback( s );
    if ( ( nullptr != timeSpec ) && ( nullptr != s->tdef ) ) {
        timeoutPerformSpecifiedActions( s, fsm::signalID::SIGNAL_EXIT );
    }
    s->parent->lastRunningChild = s;

    return s->parent;
}
/*============================================================================*/
void stateMachine::stateOnEntry( fsm::state *s ) noexcept
{
    const fsm::signal_t SIG_MSG_ENTRY = { fsm::signalID::SIGNAL_ENTRY, nullptr };

    prepareHandler( SIG_MSG_ENTRY, s );
    (void)invokeStateCallback( s );

    if ( ( nullptr != timeSpec ) && ( nullptr != s->tdef ) ) {
        timeoutPerformSpecifiedActions( s, fsm::signalID::SIGNAL_ENTRY );
    }
}
/*============================================================================*/
fsm::state* stateMachine::stateOnStart( fsm::state *s ) noexcept
{
    const fsm::signal_t SIG_MSG_START = { fsm::signalID::SIGNAL_START, nullptr };

    prepareHandler( SIG_MSG_START, s );
    (void)invokeStateCallback( s );

    if ( nullptr != fsm::_Handler::StartState ) {
        /*changes from callback takes more precedence*/
        next = fsm::_Handler::StartState;
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
fsm::status stateMachine::stateOnSignal( fsm::state *s, fsm::signal_t sig ) noexcept
{
    fsm::status status;

    prepareHandler( sig, s );
    if ( nullptr != s->tTable ) {
        /*evaluate the transition table if available*/
        /*cstat -CERT-EXP39-C_d*/
        s->sweepTransitionTable( *static_cast<fsm::_Handler*>( this ) );
        /*cstat +CERT-EXP39-C_d*/
    }
    status = invokeStateCallback( s );

    if ( nullptr != fsm::_Handler::NextState ) { /*perform the transition if available*/
        transition( fsm::_Handler::NextState, fsm::_Handler::TransitionHistory );
        /*the signal is assumed to be handled if the transition occurs*/
        status = fsm::status::SIGNAL_HANDLED;
    }

    return status;
}
/*============================================================================*/
void stateMachine::tracePathAndRetraceEntry( fsm::state **trace ) noexcept
{
    fsm::state *s;
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
void stateMachine::traceOnStart( fsm::state **entryPath ) noexcept
{
    while ( nullptr != stateOnStart( current ) ) {
        tracePathAndRetraceEntry( entryPath );
    }
}
/*============================================================================*/
fsm::signal_t stateMachine::checkForSignals( fsm::signal_t sig ) noexcept
{
    fsm::signal_t xSignal = sig;

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
            fsm::signal_t receivedSignal;

            if ( sQueue->receive( &receivedSignal ) ) {
                xSignal = receivedSignal;
            }
        }
    #endif
    /*A signal coming from the signal-queue has the higher precedence.*/
    if ( ( fsm::signalID::SIGNAL_NONE == xSignal.id ) && ( fsm::signalID::SIGNAL_NONE != signalNot.id ) ) {
        xSignal = signalNot; /*exclusion variable*/
        signalNot.id = fsm::signalID::SIGNAL_NONE;
        signalNot.data = nullptr;
    }

    return xSignal;
}
/*============================================================================*/
bool stateMachine::run( fsm::signal_t sig ) noexcept
{
    bool retValue = false;
    fsm::state *entryPath[ Q_FSM_MAX_NEST_DEPTH ];

    sig = checkForSignals( sig );
    /*Enter here only once to start the top state*/
    if ( nullptr == current ) {
        current = &top;
        next = nullptr;
        stateOnEntry( current );
        traceOnStart( entryPath );
    }
    /*evaluate the hierarchy until the signal is handled*/
    /*cstat -MISRAC++2008-6-5-2*/
    for ( fsm::state *s = current ; nullptr != s ; s = s->parent ) {
        source = s; /* level of outermost event handler */
        if ( fsm::status::SIGNAL_HANDLED == stateOnSignal( s, sig ) ) {
            if ( nullptr != next ) {  /* state transition taken? */
                /*
                 execute entry/start actions in the rest of the hierarchy
                 after transition
                 */
                tracePathAndRetraceEntry( entryPath );
                traceOnStart( entryPath );
            }
            retValue = true;
            break;/* signal processed */
        }
    }
    /*cstat +MISRAC++2008-6-5-2*/
    return retValue;
}
/*============================================================================*/
