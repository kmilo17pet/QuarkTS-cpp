#ifndef QOS_CPP_FSM
#define QOS_CPP_FSM

#include "include/types.hpp"
#include "include/timer.hpp"
#include "include/queue.hpp"

namespace qOS {

    class stateMachine;
    
    namespace sm {
        class state;

        /**
        * @brief The type for signal ID.
        */
        enum signalID : uint32_t {
            SIGNAL_START = 0xFFFFFFFFuL,
            SIGNAL_EXIT = 0xFFFFFFFEuL,
            SIGNAL_ENTRY = 0xFFFFFFFDuL,
            SIGNAL_NONE = 0xFFFFFFFCuL,
            MAX_SIGNAL = 0xFFFFFFFBuL,
            MIN_SIGNAL = 0x0uL,
            TM_MAX = 0xFFFFFFFBuL,
            TM_MIN = TM_MAX - ( Q_FSM_MAX_TIMEOUTS - 1 ),
        };

        /**
        * @brief Built-in signal to indicate that a timeout expiration event occurs.
        * @param iTm The index of the timeout (0, 1, 2... ( @c Q_FSM_MAX_TIMEOUTS-1 ) )
        * @return The built-int timeout signal at index @a iTm.
        */
        constexpr signalID SIGNAL_TIMEOUT( index_t iTm ) 
        {
            return static_cast<signalID>( signalID::TM_MAX - static_cast<sm::signalID>( Q_FSM_MAX_TIMEOUTS - 1 ) + static_cast<sm::signalID>( iTm ) );
        }

        struct _signal_s {
            signalID id{ signalID::SIGNAL_NONE };
            void *data{ nullptr };
        };
        /**
         * @brief The type to be used as a container variable for a signal.
         */
        using signal_t = struct _signal_s;

        enum status : int16_t {
            BEFORE_ANY = -32767,
            ABSENT = -32766,
            FAILURE = -32765,
            SUCCESS = -32764,
            SIGNAL_HANDLED = -32763,
        };

        /**
        * @brief This enumeration defines the possible modes to perform a
        * transition to history
        */
        enum class historyMode : uint8_t {
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
                void nextState( state &s, historyMode m = historyMode::NO_HISTORY ) noexcept
                {
                    NextState = &s;
                    TransitionHistory = m;
                }
                bool timeoutSet( const index_t i, const qOS::time_t t ) noexcept;
                bool timeoutStop( const index_t i ) noexcept;
                state& thisState( void ) noexcept
                {
                    return *State;
                }
                stateMachine& thisMachine( void ) noexcept
                { 
                    return *Machine;
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

        /**
        * @brief Pointer to a function that represents a state callback
        * @code{.c}
        * sm::status myState_Callback( sm::handler_t h ) {
        *       sm::status retVal = sm::status::SUCCESS;
        *       switch ( h.signal() ) {
        *           case sm::SIGNAL_ENTRY:
        *               break;
        *           case sm::SIGNAL_EXIT:
        *               break;
        *           case USER_SIGNAL:
        *               retVal = sm::status::SIGNAL_HANDLED;
        *               break;
        *           default:
        *               break;
        *       return retVal;
        * }
        * @endcode
        * @param h The object that handles the state machine
        * @return The execution status of the state.
        */
        using stateCallback_t = status (*)( handler_t );

        /**
        * @brief Pointer to a function that represents a surrounding callback
        * @code{.c}
        * void surrounding_Callback( sm::handler_t h ) {
        *       switch ( h.lastStatus() ) {
        *           case sm::status::BEFORE_ANY:
        *               break;
        *           case sm::status::SUCCESS:
        *               break;
        *           case sm::status::FAILURE:
        *               break;
        *           default:
        *               break;
        * }
        * @endcode
        * @param h The object that handles the state machine
        * @return none.
    */
        using surroundingCallback_t = void (*)( handler_t );

        /**
        * @brief Pointer to a function that represents a signal guard/action
        * @code{.c}
        * bool signalAction_Callback( sm::handler_t h ) {
        *       bool retVal = false
        *       if ( ConditionGuard ) {
        *           performSomeAction();
        *           retVal = true;
        *       }
        *       return retVal;
        * }
        * @endcode
        * @param h The object that handles the state machine
        * @return A boolean value that allow or rejects the state transition. If the
        * value is false the event-signal is rejected, preventing the state
        * transition to be performed.
        */
        using signalAction_t = bool (*)( handler_t );

        /**
        * @brief A typedef to hold the timeout specification options.
        */
        using timeoutSpecOption_t = uint32_t;

        struct _timeoutStateDefinition_s{
            qOS::time_t xTimeout;
            timeoutSpecOption_t options;
        };
        /**
        * @brief This type should be used to define an item for a 
        * timeout-specification table.
        */
        using timeoutStateDefinition_t = struct _timeoutStateDefinition_s;

        struct _transition_s {
            signalID xSignal{ signalID::SIGNAL_NONE };
            signalAction_t guard{ nullptr };
            state *nextState{ nullptr };
            historyMode history{ historyMode::NO_HISTORY };
            void *signalData{ nullptr };
        };
        /**
        * @brief This structure should be used to define an item for a state
        * transition table.
        */
        using transition_t = struct _transition_s;

        /**
        * @brief A state object
        * @details States are represented as instances of the state object.
        * One important attribute of this object is the callback function, which is
        * used to describe the behavior specific to the state. Also there is a
        * pointer to the parent state to define nesting of the state and its place
        * in the hierarchical topology.
        *
        * So concrete state machine are built by adding an arbitrary number states
        * and defining callback functions.
        *
        * State  machines  are constructed by composition, therefore, the topology
        * of a state machine is determined upon construction. In this FSM
        * implementation, there are not distinction between composite states(states
        * containing sub-states) and leaf states. All states are potentially
        * composite.
        *
        * The APIs stateMachine::add() and state::add()  should be used to 
        * initialize the state and define its position in the topology.
        */
        class state {
            private:
                state *parent{ nullptr };
                state *lastRunningChild{ nullptr };
                state *initState{ nullptr };
                stateCallback_t sCallback{ nullptr};
                timeoutStateDefinition_t *tdef{ nullptr };
                transition_t *tTable{ nullptr };
                void *sData{ nullptr };
                size_t tEntries{ 0u };
                size_t nTm{ 0u };
                void sweepTransitionTable( _Handler &h ) noexcept;
                state( state const& ) = delete;
                void operator=( state const& ) = delete;
                bool subscribe( sm::state *s, const sm::stateCallback_t sFcn, sm::state *init ) noexcept;
                void topSelf( const sm::stateCallback_t topFcn, sm::state *init ) noexcept;
            public:
                state() = default;
                /**
                * @brief Add the specified state as a child state
                * @param[in] s The state object.
                * @param[in] sFcn The handler function associated to the state.
                *
                * Prototype: @code sm::status xCallback( sm::handler_t h ) @endcode
                * @param[in] init The first child-state to be executed if the subscribed
                * state its a parent in an hierarchical pattern. (default transition).
                * You can ignore this argument.
                * @return true on success, otherwise return false.
                */
                bool add( sm::state &s, sm::stateCallback_t sFcn, sm::state &init ) noexcept
                {
                    return subscribe( &s, sFcn, &init );
                }
                /**
                * @brief Add the specified state as a child state
                * @param[in] s The state object.
                * @param[in] sFcn The handler function associated to the state.
                *
                * Prototype: @code sm::status xCallback( sm::handler_t h ) @endcode
                * @return true on success, otherwise return false.
                */
                bool add( sm::state &s, sm::stateCallback_t sFcn ) noexcept
                {
                    return subscribe( &s, sFcn, nullptr );
                }
                /**
                * @brief Installs a table with the outgoing transitions for this
                * state
                * @param[in] table An array of entries of type qSM_Transition_t with the
                * outgoing transitions. Each entry relates signals, actions and the target
                * state using the following layout:
                * @verbatim { [Signal], [Action/Guard], [Target state], [History Mode] } @endverbatim
                * @param[in] n The number of elements inside @a table.
                * @return true on success, otherwise return false.
                */
                bool setTransitions( transition_t *table, size_t n ) noexcept;
                /**
                * @brief Setup fixed timeouts for the specified state using a lookup-table.
                * @attention This feature its only available if the FSM has a signal-queue
                * installed.
                * @pre The container state-machine must have a timeout-specification
                * installed.
                * @note The lookup table should be an array of type
                * timeoutStateDefinition_t with @a n elements matching { time, options }.
                * @see stateMachine::installSignalQueue(), stateMachine::installTimeoutSpec()
                * @param[in] tdef  The lookup table matching the requested timeout values
                * with their respective options.
                * @verbatim { [Timeout value], [Options(Combined with a bitwise OR)] } @endverbatim
                * @param[in] n The number of elements inside @a tdef.
                * @return Returns true on success, otherwise returns false.
                */
                bool setTimeouts( timeoutStateDefinition_t *def, size_t n ) noexcept;
                void* getData( void ) noexcept;
                void setData( void *pData ) noexcept;
                transition_t* getTransitionTable( void ) noexcept;
                void setCallback( const stateCallback_t sFcn ) noexcept;
            friend class qOS::stateMachine;
        };

        /**
        * @brief A FSM Timeout-specification object
        * @note Do not access any member of this structure directly.
        */
        class timeoutSpec {
            uint32_t isPeriodic{ 0u };
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
            size_t sig_slot;
            size_t sub_slot;
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

    /**
    * @brief A FSM(Finite State Machine) object
    * @details In QuarkTS++, a state-machine must be instantiated with an object
    * of type stateMachine.
    *
    * Like any other OS object, a Finite State Machine (FSM) must be explicitly
    * initialized before it can be used. The stateMachine::setup() initializes
    * the instance, sets the callback for the top state, sets the initial state
    * and the surrounding callback function.
    */
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
            uint8_t levelsToLCA( sm::state *target ) noexcept;
            void exitUpToLCA( uint8_t lca ) noexcept;
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
            /**
            * @brief Initializes a Finite State Machine (FSM).
            * @see core::addStateMachineTask()
            * @note This API also initializes the top state.
            * @param[in] topFcn The callback for the "Top" state.
            * @param[in] init The first state to be executed (init-state or default
            * transition).
            * @param[in] sFcn The surrounding callback. To ignore pass @c nullptr.
            * @param[in] pData Represents the FSM arguments. User storage pointer. To
            * ignore pass @c nullptr.
            * @return Returns true on Success, otherwise returns false.
            */
            inline bool setup( sm::stateCallback_t topFcn, sm::state &init, sm::surroundingCallback_t sFcn = nullptr, void* pData = nullptr)  noexcept
            {
                return setup( topFcn, &init, sFcn, pData );
            }
            /**
            * @brief Add the specified state to the stateMachine "Top" state
            * @param[in] s The state object.
            * @param[in] parent A pointer to the parent state (Use #QSM_STATE_TOP) if
            * the parent is the top state.
            * @param[in] sFcn The handler function associated to the state.
            *
            * Prototype: @code sm::status xCallback( sm::handler_t h ) @endcode
            * @param[in] init The first child-state to be executed if the subscribed
            * state its a parent in an hierarchical pattern. (default transition).
            * You can ignore this argument.
            * @return true on success, otherwise return false.
            */
            inline bool add( sm::state &s, sm::stateCallback_t sFcn, sm::state &init ) noexcept
            {
                return top.subscribe( &s, sFcn, &init );
            }
            /**
            * @brief This function adds the specified state to the stateMachine
            * "Top" state
            * @param[in] s The state object.
            * @param[in] parent A pointer to the parent state (Use #QSM_STATE_TOP) if
            * the parent is the top state.
            * @param[in] sFcn The handler function associated to the state.
            *
            * Prototype: @code sm::status xCallback( sm::handler_t h ) @endcode
            * @return true on success, otherwise return false.
            */
            inline bool add( sm::state &s, sm::stateCallback_t sFcn ) noexcept
            {
                return top.subscribe( &s, sFcn, nullptr );
            }
            /**
            * @brief Install a signal queue to the provided Finite State Machine (FSM).
            * @pre Queue object should be previously initialized by using
            * queue::setup()
            * @attention Queue item size = sizeof( @ref qSM_Signal_t )
            * @param[in] q The queue to be installed.
            * @return true on success, otherwise return false.
            */
            bool installSignalQueue( queue& q ) noexcept;
            /**
            * @brief Sends a signal to a state machine 
            * @note If the signal queue is not available, an exclusion variable will be
            * used.This means that the signal cannot be sent until the variable is empty.
            * (the signal was handled by the state-machine engine).
            * @warning Data associated to the signal is not deep-copied to a queue or any 
            * buffer. It's only data pointer (address in memory) that is shallow-copied
            * to a signal queue so it has to point to a globally accessible memory. 
            * If it pointed to a sender's local variable (from the stack) it would be 
            * invalid after sender returns from the function that sends the signal.
            * @note The signal-queue event has the highest precedence.
            * @param[in] sig The user-defined signal.
            * @param[in] sData The data associated to the signal.
            * @param[in] isUrgent If true, the signal will be sent to the front of the
            * queue. (only if the there is a signal-queue available)
            * @return true if the provided signal was successfully delivered to the
            * state machine , otherwise return false. false if there is a queue,
            * and the signal cannot be inserted because it is full.
            */
            bool sendSignal( sm::signalID sig, void *sData = nullptr, bool isUrgent = false ) noexcept;
            /**
            * @brief Sends a signal to all its subscribers (if available).
            * @note If the signal queue is not available, an exclusion variable will be
            * used.This means that the signal cannot be sent until the variable is empty.
            * (the signal was handled by the state-machine engine).
            * @remark To enable the functionality of sending signals to subscribers, you
            * must set the macros #Q_FSM_PS_SIGNALS_MAX and #Q_FSM_PS_SUB_PER_SIGNAL_MAX
            * in the configuration file @c qconfig.h
            * @warning Data associated to the signal is not deep-copied to a queue or any 
            * buffer. It's only data pointer (address in memory) that is shallow-copied
            * to a signal queue so it has to point to a globally accessible memory. 
            * If it pointed to a sender's local variable (from the stack) it would be 
            * invalid after sender returns from the function that sends the signal.
            * @note The signal-queue event has the highest precedence.
            * @param[in] sig The user-defined signal.
            * @param[in] sData The data associated to the signal.
            * @param[in] isUrgent If true, the signal will be sent to the front of the
            * queue. (only if the there is a signal-queue available)
            * @return true if the provided signal was successfully delivered to the
            * subscribers (if available), otherwise return false. false if there
            * is a queue, and the signal cannot be inserted because it is full.
            */
            bool sendSignalToSubscribers( sm::signalID sig, void *sData = nullptr, bool isUrgent = false ) noexcept;
            /**
            * @brief Install the Timeout-specification object to target FSM to allow
            * timed signals within states.
            * @attention This feature its only available if the FSM has a signal-queue
            * installed.
            * @pre This feature depends on the @ref timers extension. Make sure the
            * time base is functional.
            * @note You can increase the number of available timeouts instances by
            * changing the @c Q_FSM_MAX_TIMEOUTS configuration macro inside @c qconfig.h
            * @see stateMachine::installSignalQueue()
            * @param[in] ts The timeout specification object.
            * @return Returns true on success, otherwise returns false.
            */
            bool installTimeoutSpec( sm::timeoutSpec &ts ) noexcept;
            /**
            * @brief Set the time for the selected built-in timeout inside the target FSM.
            * @pre Requires an installed timeout-specification.
            * For this use stateMachine::installTimeoutSpec()
            * @pre Requires an installed signal-queue.
            * For this use stateMachine::installSignalQueue()
            * @param[in] xTimeout The index of the requested timeout
            * (0, 1, 2 ... (@c Q_FSM_MAX_TIMEOUTS-1) )
            * @param[in] t The specified time.
            * @return Returns true on success, otherwise returns false.
            */
            bool timeoutSet( const index_t xTimeout, const qOS::time_t t ) noexcept;
            /**
            * @brief Stop the time count for the selected built-in timeout.
            * @pre Requires an installed timeout-specification.
            * For this use stateMachine::installTimeoutSpec()
            * @pre Requires an installed signal-queue.
            * For this use stateMachine::installSignalQueue()
            * @param[in] xTimeout The index of the timeout
            * (0, 1, 2 ... (@c Q_FSM_MAX_TIMEOUTS-1) )
            * @return Returns true on success, otherwise returns false.
            */
            bool timeoutStop( const index_t xTimeout ) noexcept;
            const sm::state& getTop( void ) const noexcept;
            sm::state * const & getCurrent( void ) const noexcept;
            queue * const & getQueue( void ) const noexcept;
            sm::timeoutSpec * const & getTimeSpec( void ) const noexcept;
            void * const & getData( void ) const noexcept;
            void setSurrounding( const sm::surroundingCallback_t sFcn ) noexcept;
            /**
            * @brief Subscribe state machine to a particular signal
            * @pre Subscriber FSM should be previously initalized with
            * stateMachine::setup()
            * @param[in] s Signal ID to which the subscriber FSM wants to subscribe.
            * @return Returns true on success, otherwise returns false.
            */
            bool subscribeToSignal( sm::signalID s ) noexcept;
            /**
            * @brief Unsubscribe state machine from a particular signal
            * @pre Subscriber FSM should be previously initalized with
            * stateMachine::setup()
            * @param[in] s Signal ID to which the subscriber FSM wants to unsubscribe.
            * @return Returns true on success, otherwise returns false.
            */
            bool unsubscribeFromSignal( sm::signalID s ) noexcept;
            /**
            * @brief Execute the Finite State Machine (FSM).
            * @see core::addStateMachineTask()
            * @param[in] sig User-defined signal (this value will be ignored if the
            * installed queue has items available)
            * @note A signal coming from the signal-queue has the higher precedence.
            * The user-defined signal can be overridden
            * @return true if the signal was successfully handled, otherwise returns
            * false.
            */
            bool run( sm::signal_t sig ) noexcept;
        friend class core;
    };

}

#endif /*QOS_CPP_FSM*/
