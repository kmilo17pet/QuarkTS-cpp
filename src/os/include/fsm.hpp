#ifndef QOS_CPP_FSM
#define QOS_CPP_FSM

#include "types.hpp"
#include "timer.hpp"
#include "queue.hpp"

namespace qOS {

    class stateMachine;
    
    namespace sm {
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
            return static_cast<signalID>( signalID::TM_MAX - static_cast<sm::signalID>( Q_FSM_MAX_TIMEOUTS - 1 ) + static_cast<sm::signalID>( iTm ) );
        }

        struct _signal_s {
            signalID id{ signalID::SIGNAL_NONE };
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
                signalID Signal{ signalID::SIGNAL_NONE };
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
            friend class qOS::sm::state;
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
            signalID xSignal{ signalID::SIGNAL_NONE };
            signalAction_t guard{ nullptr };
            state *nextState{ nullptr };
            historyMode history{ historyMode::NO_HISTORY };
            void *signalData{ nullptr };
        };
        using transition_t = struct _transition_s;

        class state {
            private:
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
                bool subscribe( sm::state *s, const sm::stateCallback_t sFcn, sm::state *init ) noexcept;
                void topSelf( const sm::stateCallback_t topFcn, sm::state *init ) noexcept;
            public:
                state() = default;
                bool add( sm::state &s, sm::stateCallback_t sFcn, sm::state &init ) noexcept
                {
                    return subscribe( &s, sFcn, &init );
                }
                bool add( sm::state &s, sm::stateCallback_t sFcn ) noexcept
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
                void setCallback( const stateCallback_t sFcn ) noexcept;
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
            std::size_t sig_slot;
            std::size_t sub_slot;
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

    class stateMachine : protected sm::_Handler {
        private:
            sm::state *current{ nullptr };
            sm::state *next{ nullptr };
            sm::state *source{ nullptr };
            queue *sQueue{ nullptr };
            sm::timeoutSpec *timeSpec{ nullptr };
            sm::surroundingCallback_t surrounding{ nullptr };
            sm::state top;
            sm::signal_t signalNot;
            void *owner{ nullptr };
            void unsubscribeAll( void ) noexcept;
            bool internalSignalSend( sm::signalID sig, void *sData, bool isUrgent ) noexcept;
            void timeoutCheckSignals( void ) noexcept;
            void timeoutPerformSpecifiedActions( sm::state * const s, sm::signalID sig ) noexcept;
            sm::psIndex_t getSubscriptionStatus( sm::signalID s ) noexcept;
            void transition( sm::state *target, sm::historyMode mHistory ) noexcept;
            std::uint8_t levelsToLCA( sm::state *target ) noexcept;
            void exitUpToLCA( std::uint8_t lca ) noexcept;
            void prepareHandler( sm::signal_t sig, sm::state *s ) noexcept;
            sm::status invokeStateCallback( sm::state * const s ) noexcept;
            sm::state* stateOnExit( sm::state *s ) noexcept;
            void stateOnEntry( sm::state *s ) noexcept;
            sm::state* stateOnStart( sm::state *s ) noexcept;
            sm::status stateOnSignal( sm::state *s, sm::signal_t sig ) noexcept;
            void tracePathAndRetraceEntry( sm::state **trace ) noexcept;
            void traceOnStart( sm::state **entryPath ) noexcept;
            sm::signal_t checkForSignals( sm::signal_t sig ) noexcept;
            stateMachine( stateMachine const& ) = delete;
            void operator=( stateMachine const& ) = delete;
            bool setup( sm::stateCallback_t topFcn, sm::state *init, const sm::surroundingCallback_t sFcn, void* pData ) noexcept;
        public:
            void *mData{ nullptr };
            stateMachine() = default;
            inline bool setup( sm::stateCallback_t topFcn, sm::state &init, sm::surroundingCallback_t sFcn, void* pData )  noexcept
            {
                return setup( topFcn, &init, sFcn, pData );
            }
            inline bool setup( sm::stateCallback_t topFcn, sm::state &init, sm::surroundingCallback_t sFcn )  noexcept
            {
                return setup( topFcn, &init, sFcn, nullptr );
            }
            inline bool setup( sm::stateCallback_t topFcn, sm::state &init ) noexcept
            {
                return setup( topFcn, &init, nullptr, nullptr );
            }
            inline bool add( sm::state &s, sm::stateCallback_t sFcn, sm::state &init ) noexcept
            {
                return top.subscribe( &s, sFcn, &init );
            }
            inline bool add( sm::state &s, sm::stateCallback_t sFcn ) noexcept
            {
                return top.subscribe( &s, sFcn, nullptr );
            }
            bool installSignalQueue( queue& q ) noexcept;
            bool sendSignal( sm::signalID sig, void *sData, bool isUrgent ) noexcept;
            bool sendSignal( sm::signalID sig, void *sData ) noexcept
            {
                return sendSignal( sig, sData, false );
            }
            inline bool sendSignal( sm::signalID sig ) noexcept
            {
                return sendSignal( sig, nullptr, false );
            }
            bool sendSignalToSubscribers( sm::signalID sig, void *sData, bool isUrgent ) noexcept;
            bool installTimeoutSpec( sm::timeoutSpec &ts ) noexcept;
            bool timeoutSet( index_t xTimeout, qOS::time_t t ) noexcept;
            bool timeoutStop( index_t xTimeout ) noexcept;
            const sm::state& getTop( void ) const noexcept;
            sm::state * const & getCurrent( void ) const noexcept;
            queue * const & getQueue( void ) const noexcept;
            sm::timeoutSpec * const & getTimeSpec( void ) const noexcept;
            void * const & getData( void ) const noexcept;
            void setSurrounding( const sm::surroundingCallback_t sFcn ) noexcept;
            bool subscribeToSignal( sm::signalID s ) noexcept;
            bool unsubscribeFromSignal( sm::signalID s ) noexcept;
            bool run( sm::signal_t sig ) noexcept;
        friend class core;
    };

}

#endif /*QOS_CPP_FSM*/
