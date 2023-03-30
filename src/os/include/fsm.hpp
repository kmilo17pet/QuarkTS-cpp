#ifndef QOS_CPP_FSML
#define QOS_CPP_FSML

#include "types.hpp"
#include "timer.hpp"
#include "queue.hpp"

namespace qOS {

    class stateMachine;
    
    namespace fsm {
        class state;

        enum signalID : std::uint32_t {
            SIGNAL_START = 0xFFFFFFFFuL,
            SIGNAL_EXIT = 0xFFFFFFFEuL,
            SIGNAL_ENTRY = 0xFFFFFFFDuL,
            SIGNAL_NONE = 0xFFFFFFFCuL,
            MAX_SIGNAL = 0xFFFFFFFBuL,
            MIN_SIGNAL = 0x0uL,
            TM_MAX = 0xFFFFFFFBuL,
            TM_MIN = TM_MAX - ( Q_FSM_MAX_TIMEOUTS - 1 ),
        };

        constexpr signalID SIGNAL_TIMEOUT( index_t iTm ) 
        {
            return static_cast<signalID>( signalID::TM_MAX - static_cast<fsm::signalID>( Q_FSM_MAX_TIMEOUTS - 1 ) + static_cast<fsm::signalID>( iTm ) );
        }

        struct _signal_s {
            signalID id;
            void *data{ nullptr };
        };
        using signal_t = struct _signal_s;

        enum status : std::int16_t {
            BEFORE_ANY = -32767,
            ABSENT = -32766,
            FAILURE = -32765,
            SUCCESS = -32764,
            SIGNAL_HANDLED = -32763,
        };

        enum class historyMode : std::uint8_t {
            NO_HISTORY = 0u,
            SHALLOW_HISTORY,
            DEEP_HISTORY,
        };

        class _Handler {
            protected:
                state *StartState{ nullptr };
                state *NextState{ nullptr };
                stateMachine* Machine{ nullptr };
                state *State{ nullptr };
                historyMode TransitionHistory{ historyMode::NO_HISTORY };
                status Status{ SUCCESS };
                signalID Signal;
                _Handler( _Handler const& ) = delete;      /* not copyable*/
                void operator=( _Handler const& ) = delete;  /* not assignable*/
            public:
                _Handler() = default;
                void *SignalData{ nullptr };
                void *Data{ nullptr };
                void *StateData{ nullptr };
                void nextState( state &s ) noexcept
                {
                    NextState = &s;
                }
                void nextState( state *s, historyMode m ) noexcept
                {
                    NextState = s;
                    TransitionHistory = m;
                }
                bool timeoutSet( index_t i, qOS::time_t t ) noexcept;
                bool timeoutStop( index_t i ) noexcept;
                state* thisState( void ) noexcept
                {
                    return State;
                }
                stateMachine* thisMachine( void ) noexcept
                { 
                    return Machine;
                }
                signalID signal( void ) const noexcept
                {
                    return Signal;
                }
                status lastStatus( void ) const noexcept
                {
                    return Status;
                }
            friend class qOS::fsm::state;
        };
        using handler_t = _Handler&;
        using stateCallback_t = status (*)( handler_t );
        using surroundingCallback_t = void (*)( handler_t );
        using signalAction_t = bool (*)( handler_t );
        using timeoutSpecOption_t = std::uint32_t;

        struct _timeoutStateDefinition_s{
            qOS::time_t xTimeout;
            timeoutSpecOption_t options;
        };
        using timeoutStateDefinition_t = struct _timeoutStateDefinition_s;

        struct _transition_s {
            signalID xSignal;
            signalAction_t guard{ nullptr };
            state *nextState{ nullptr };
            historyMode history{ historyMode::NO_HISTORY };
            void *signalData;
        };
        using transition_t = struct _transition_s;

        class state {
            protected:
                state *parent{ nullptr };
                state *lastRunningChild{ nullptr };
                state *initState{ nullptr };
                stateCallback_t sCallback{ nullptr};
                timeoutStateDefinition_t *tdef{ nullptr };
                transition_t *tTable{ nullptr };
                void *sData{ nullptr };
                std::size_t tEntries{ 0u };
                std::size_t nTm{ 0u };
                void sweepTransitionTable( _Handler &h ) noexcept;
                state( state const& ) = delete;
                void operator=( state const& ) = delete;
                bool subscribe( fsm::state *s, fsm::stateCallback_t sFcn, fsm::state *init ) noexcept;
                void topSelf( fsm::stateCallback_t topFcn, fsm::state *init ) noexcept;
            public:
                state() = default;
                bool add( fsm::state &s, fsm::stateCallback_t sFcn, fsm::state &init ) noexcept
                {
                    return subscribe( &s, sFcn, &init );
                }
                bool add( fsm::state &s, fsm::stateCallback_t sFcn ) noexcept
                {
                    return subscribe( &s, sFcn, nullptr );
                }
                bool setTransitions( transition_t *table, std::size_t n ) noexcept;
                bool setTimeouts( timeoutStateDefinition_t *def, std::size_t n ) noexcept;
                state* getInit( void ) noexcept;
                state* getLastState( void ) noexcept;
                state* getParent( void ) noexcept;
                void* getData( void ) noexcept;
                void setData( void *pData ) noexcept;
                transition_t* getTransitionTable( void ) noexcept;
                void setCallback( stateCallback_t sFcn ) noexcept;
            friend class qOS::stateMachine;
        };

        class timeoutSpec {
            std::uint32_t isPeriodic{ 0u };
            timer timeout[ Q_FSM_MAX_TIMEOUTS ];
            friend class qOS::stateMachine;
        };

        enum psReqStatus{
            PS_SIGNAL_NOT_FOUND,
            PS_SUBSCRIBER_NOT_FOUND,
            PS_SUBSCRIBER_FOUND,
            PS_SIGNAL_SLOTS_FULL,
            PS_SUBSCRIBER_SLOTS_FULL
        };

        struct _psIndex_s {
            psReqStatus status;
            std::size_t sig_slot, sub_slot;
        };
        using psIndex_t = _psIndex_s;

        extern const timeoutSpecOption_t TIMEOUT_SET_ENTRY;
        extern const timeoutSpecOption_t TIMEOUT_RST_ENTRY;
        extern const timeoutSpecOption_t TIMEOUT_SET_EXIT;
        extern const timeoutSpecOption_t TIMEOUT_RST_EXIT;
        extern const timeoutSpecOption_t TIMEOUT_KEEP_IF_SET;
        extern const timeoutSpecOption_t TIMEOUT_PERIODIC;
        constexpr timeoutSpecOption_t TIMEOUT_INDEX( index_t i )
        {
            return ( 0x00FFFFFFuL & static_cast<timeoutSpecOption_t>( i ) );
        }

    }

    class stateMachine : protected fsm::_Handler {
        protected:
            fsm::state *current{ nullptr };
            fsm::state *next{ nullptr };
            fsm::state *source{ nullptr };
            queue *sQueue{ nullptr };
            fsm::timeoutSpec *timeSpec{ nullptr };
            fsm::surroundingCallback_t surrounding{ nullptr };
            fsm::state top;
            fsm::signal_t signalNot;
            void *owner{ nullptr };
            void unsubscribeAll( void ) noexcept;
            bool internalSignalSend( fsm::signalID sig, void *sData, bool isUrgent ) noexcept;
            void timeoutCheckSignals( void ) noexcept;
            void timeoutPerformSpecifiedActions( fsm::state * const s, fsm::signalID sig ) noexcept;
            fsm::psIndex_t getSubscriptionStatus( fsm::signalID s ) noexcept;
            void transition( fsm::state *target, fsm::historyMode mHistory ) noexcept;
            std::uint8_t levelsToLCA( fsm::state *target ) noexcept;
            void exitUpToLCA( std::uint8_t lca ) noexcept;
            void prepareHandler( fsm::signal_t sig, fsm::state *s ) noexcept;
            fsm::status invokeStateCallback( fsm::state * const s ) noexcept;
            fsm::state* stateOnExit( fsm::state *s ) noexcept;
            void stateOnEntry( fsm::state *s ) noexcept;
            fsm::state* stateOnStart( fsm::state *s ) noexcept;
            fsm::status stateOnSignal( fsm::state *s, fsm::signal_t sig ) noexcept;
            void tracePathAndRetraceEntry( fsm::state **trace ) noexcept;
            void traceOnStart( fsm::state **entryPath ) noexcept;
            fsm::signal_t checkForSignals( fsm::signal_t sig ) noexcept;
            stateMachine( stateMachine const& ) = delete;
            void operator=( stateMachine const& ) = delete;
            bool setup( fsm::stateCallback_t topFcn, fsm::state *init, fsm::surroundingCallback_t sFcn, void* pData ) noexcept;
        public:
            void *mData{ nullptr };
            stateMachine() = default;
            inline bool setup( fsm::stateCallback_t topFcn, fsm::state &init, fsm::surroundingCallback_t sFcn, void* pData )  noexcept
            {
                return setup( topFcn, &init, sFcn, pData );
            }
            inline bool setup( fsm::stateCallback_t topFcn, fsm::state &init, fsm::surroundingCallback_t sFcn )  noexcept
            {
                return setup( topFcn, &init, sFcn, nullptr );
            }
            inline bool setup( fsm::stateCallback_t topFcn, fsm::state &init ) noexcept
            {
                return setup( topFcn, &init, nullptr, nullptr );
            }
            inline bool add( fsm::state &s, fsm::stateCallback_t sFcn, fsm::state &init ) noexcept
            {
                return top.subscribe( &s, sFcn, &init );
            }
            inline bool add( fsm::state &s, fsm::stateCallback_t sFcn ) noexcept
            {
                return top.subscribe( &s, sFcn, nullptr );
            }
            bool installSignalQueue( queue& q ) noexcept;
            bool sendSignal( fsm::signalID sig, void *sData, bool isUrgent ) noexcept;
            bool sendSignal( fsm::signalID sig, void *sData ) noexcept
            {
                return sendSignal( sig, sData, false );
            }
            inline bool sendSignal( fsm::signalID sig ) noexcept
            {
                return sendSignal( sig, nullptr, false );
            }
            bool sendSignalToSubscribers( fsm::signalID sig, void *sData, bool isUrgent ) noexcept;
            bool installTimeoutSpec( fsm::timeoutSpec &ts ) noexcept;
            bool timeoutSet( index_t xTimeout, qOS::time_t t ) noexcept;
            bool timeoutStop( index_t xTimeout ) noexcept;
            const fsm::state* getTop( void ) const noexcept;
            fsm::state* getCurrent( void ) noexcept;
            queue* getQueue( void ) noexcept;
            fsm::timeoutSpec* getTimeSpec( void ) noexcept;
            void* getData( void ) noexcept;
            void setSurrounding( fsm::surroundingCallback_t sFcn ) noexcept;
            bool subscribeToSignal( fsm::signalID s ) noexcept;
            bool unsubscribeFromSignal( fsm::signalID s ) noexcept;
            bool run( fsm::signal_t sig ) noexcept;
        friend class core;
    };

}

#endif /*QOS_CPP_FSML*/