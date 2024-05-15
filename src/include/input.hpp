#ifndef QOS_EDGE_CHECK
#define QOS_EDGE_CHECK

#include "include/types.hpp"
#include "include/list.hpp"
#include "include/timer.hpp"

namespace qOS {


    /** @addtogroup qinput Input channel utilities
    * @brief An interface to manage and simplify the value state (with edge-checking)
    * of incoming binary signals.
    *  @{
    */

    /**
    * @brief Input Intefaces.
    */
    namespace input {


        class watcher;

        /** @addtogroup qinput
        *  @{
        */

        /**
        * @brief An enum with all the possible states detected by the
        * qOS::input::watcher class  for a specified qOS::input::channel.
        */
        enum class state {
            FALLING_EDGE = -3,
            RISING_EDGE = -2,
            UNKNOWN = -1,
            OFF = 0,
            ON = 1,
        };

        /**
        * @brief An enum with all the possible events that can be detected by the
        * watcher class for a specified input-channel.
        */
        enum class event {
            FALLING_EDGE, /**< Event on falling-edge of the digital input-channel*/
            RISING_EDGE,  /**< Event on rising-edge of the digital input-channel*/
            STEADY_ON,    /**< Event when the digital input-channel has been kept on for a while .*/
            STEADY_OFF,   /**< Event when the digital input-channel has been kept off for a while .*/
        };

        enum class type {
            DIGITAL,
            ANALOG,
        };

        using channelReaderFcn_t = int (*)( uint8_t );
        using eventCallback_t = void(*)( uint8_t, const event );


        /**
        * @brief An digital input-channel object.
        */
        class channel : protected node {
            private:
                type channelType;
                eventCallback_t fallingCB{ nullptr };
                eventCallback_t risingCB{ nullptr };
                eventCallback_t steadyOnCB{ nullptr };
                eventCallback_t steadyOffCB{ nullptr };
                bool bSteadyOn{ false };
                bool bSteadyOff{ false };
                input::state prevState{ input::state::UNKNOWN };
                input::state state{ input::state::UNKNOWN };
                qOS::clock_t tChange{0U};
                qOS::clock_t tSteadyOn{0xFFFFFFFFU};
                qOS::clock_t tSteadyOff{0xFFFFFFFFU};
                bool negValue{ false };
                uint8_t xChannel;
                int riseThreshold;
                int fallThreshold;
                channel( channel const& ) = delete;
                void operator=( channel const& ) = delete;
                bool registerEvent( event e, eventCallback_t c, qOS::clock_t t = 1000U ) noexcept;
                inline bool unregisterEvent( event e ) noexcept
                {
                    return registerEvent( e, nullptr );
                }
            public:
                virtual ~channel() {}
                /**
                * @brief Constructor for the analog input channel instance.
                * @param[in] inputChannel The specified channel(pin) to read.
                * @param[in] upperThreshold The upper threshold value.
                * @param[in] lowerThreshold The lower threshold value.
                */
                channel( uint8_t inputChannel, int upperThreshold, int lowerThreshold ) : xChannel( inputChannel ), riseThreshold( upperThreshold ), fallThreshold( lowerThreshold )
                {
                    channelType = input::type::ANALOG;
                }
                /**
                * @brief Constructor for the digital input channel instance.
                * @param[in] inputChannel The specified channel(pin) to read.
                * @param[in] invert To invert/negate the raw-reading.
                */
                channel( uint8_t inputChannel, bool invert = false ) : negValue( invert ), xChannel( inputChannel )
                {
                    channelType = input::type::DIGITAL;
                }
                /**
                * @brief Set/Change the channel(pin) number.
                * @param[in] inputChannel The specified channel(pin) to read.
                * @return @c true on success. Otherwise @c false.
                */
                inline bool setChannel( const uint8_t inputChannel ) noexcept
                {
                    bool retValue = false;

                    if ( inputChannel < 32U ) {
                        xChannel = inputChannel;
                        retValue = true;
                    }

                    return retValue;
                }
            friend class watcher;
        };

        /**
        * @brief The digital input-channel watcher class.
        */
        class watcher {
            private:
                list nodes;
                qOS::timer waitDebounce;
                qOS::duration_t debounceTime{ 100_ms };
                channelReaderFcn_t digitalReader{ nullptr };
                channelReaderFcn_t analogReader{ nullptr };
                watcher( watcher const& ) = delete;
                void operator=( watcher const& ) = delete;
                void watchDigital( channel * const n ) noexcept;
                void watchAnalog( channel * const n ) noexcept;
                inline state read( channel * const n ) noexcept
                {
                    state s = state::UNKNOWN;
                    const state vOn = ( n->negValue ) ? state::OFF : state::ON;
                    const state vOff = ( n->negValue ) ? state::ON : state::OFF;

                    if ( nullptr != digitalReader ) {
                        s = digitalReader( n->xChannel ) ? vOn : vOff;
                    }
                    return s;
                }
            public:
                /**
                * @brief Constructor for the input-watcher instance
                * @param[in] rFcn A pointer to a function that reads the specific
                * channel
                * @param[in] timeDebounce The specified time  to bypass the
                * bounce of the digital input channels
                * @return @c true on success. Otherwise @c false.
                */
                watcher( const channelReaderFcn_t& rDigital, const channelReaderFcn_t& rAnalog, const qOS::duration_t timeDebounce ) noexcept
                {
                    debounceTime = timeDebounce;
                    digitalReader = rDigital;
                    analogReader = rAnalog;
                }
                /**
                * @brief Add a channel to the watcher instance
                * @param[in] n The input-Channel to watch
                * @return @c true on success. Otherwise @c false.
                */
                bool add( channel& n ) noexcept
                {
                    n.prevState = ( input::type::DIGITAL == n.channelType ) ? read( &n ) : input::state::OFF;
                    n.tChange = clock::getTick();
                    return nodes.insert( &n );
                }
                /**
                * @brief Remove a channel to the watcher instance
                * @param[in] n The input-Channel to watch
                * @return @c true on success. Otherwise @c false.
                */
                bool remove( channel& n ) noexcept
                {
                    return nodes.remove( &n );
                }
                /**
                * @brief Watch for the state and events for all channels
                * registered inside the watcher instance (Non-Blocking call).
                * @return @c true when the input channels have been completed an
                * updated cycle. Otherwise @c false.
                */
                bool watch( void ) noexcept;
                /**
                * @brief Register an event-callback to be activated in the
                * watcher instance where the input-channel is being added.
                * @param[in] n The input-channel instance.
                * @param[in] e The event to register.
                * @param[in] c The callback to be invoked when the event is detected.
                * @param[in] t The time associated to the event.
                * @return @c true on success. Otherwise @c false.
                */
                bool registerEvent( channel& n, event e, eventCallback_t c, qOS::clock_t t = 1000U ) noexcept
                {
                    return n.registerEvent( e, c, t );
                }
                /**
                * @brief Un-Register an event-callback for the specified
                * input-channel.
                * @param[in] n The input-channel instance.
                * @param[in] e The event to unregister.
                * @return @c true on success. Otherwise @c false.
                */
                bool unregisterEvent( channel& n, event e ) noexcept
                {
                    return n.unregisterEvent( e );
                }
                /**
                * @brief Register an event-callback to be activated in the
                * watcher instance for all the input-channels being added.
                * @param[in] e The event to register.
                * @param[in] c The callback to be invoked when the event is detected.
                * @param[in] t The time associated to the event.
                * @return @c true if operation succeeds in all input-channels.
                * Otherwise @c false.
                */
                bool registerEvent( event e, eventCallback_t c, qOS::clock_t t = 1000U ) noexcept
                {
                    bool retValue = true;
                    for ( auto i = nodes.begin(); i.untilEnd() ; i++ ) {
                        input::channel * const n = i.get<input::channel*>();
                        retValue &= n->registerEvent( e, c, t );
                    }
                    return retValue;
                }
                /**
                * @brief Un-Register an event-callback for all input-channels
                * being added.
                * @param[in] e The event to unregister.
                * @return @c true if operation succeeds in all input-channels.
                * Otherwise @c false.
                */
                bool unregisterEvent( event e ) noexcept
                {
                    bool retValue = true;
                    for ( auto i = nodes.begin(); i.untilEnd() ; i++ ) {
                        input::channel * const n = i.get<input::channel*>();
                        retValue &= n->unregisterEvent( e );
                    }
                    return retValue;
                }
        };

        /** @}*/
    }

    /** @}*/
}

#endif /*QOS_EDGE_CHECK*/