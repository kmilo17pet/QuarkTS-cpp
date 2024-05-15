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


        class observer;

        /** @addtogroup qinput
        *  @{
        */

        /**
        * @brief An enum with all the possible states detected by the
        * qOS::input::observer class  for a specified qOS::input::channel.
        */
        enum class state {
            UNKNOWN = -1,
            OFF = 0,
            ON = 1,
            RISING_EDGE = 2,
            FALLING_EDGE = 3,
        };

        /**
        * @brief An enum with all the possible events that can be detected by the
        * qOS::input::observer class for a specified qOS::input::channel.
        */
        enum class event {
            FALLING_EDGE, /**< Event on falling-edge of the digital input-channel*/
            RISING_EDGE,  /**< Event on rising-edge of the digital input-channel*/
            STEADY_ON,    /**< Event when the digital input-channel has been kept on for a while .*/
            STEADY_OFF,   /**< Event when the digital input-channel has been kept off for a while .*/
        };

        using channelReaderFcn_t = int (*)( uint8_t );
        using eventCallback_t = void(*)( uint8_t, const event );

        /**
        * @brief An input channel object.
        */
        class channel : protected node {
            private:
                eventCallback_t fallingCB{ nullptr };
                eventCallback_t risingCB{ nullptr };
                eventCallback_t steadyOnCB{ nullptr };
                eventCallback_t steadyOffCB{ nullptr };
                bool bSteadyOn{ false };
                bool bSteadyOff{ false };
                input::state prevPinValue{ input::state::UNKNOWN };
                input::state state{ input::state::UNKNOWN };
                input::state current{ input::state::UNKNOWN };
                qOS::clock_t tChange{0U};
                qOS::clock_t tSteadyOn{0xFFFFFFFFU};
                qOS::clock_t tSteadyOff{0xFFFFFFFFU};
                bool negValue{ false };
                uint8_t xChannel;
                channel( channel const& ) = delete;
                void operator=( channel const& ) = delete;
            public:
                /**
                * @brief Constructor for the input channel instance.
                * @param[in] inputChannel The specified channel(pin) to read.
                * @param[in] invert To invert/negate the raw-reading.
                * @param[in] portAddress To read the input addressThe specified Pin to read.
                */
                channel( uint8_t inputChannel, bool invert = false ) : negValue( invert ), xChannel( inputChannel ) {}
                virtual ~channel() {}
                bool setCallback( event e, eventCallback_t c, qOS::clock_t t = 1000U );
                /**
                * @brief Query the state of the specified channel.
                * @return The state of the input channel.
                */
                inline input::state getState( void ) const noexcept
                {
                    return state;
                }
                /**
                * @brief Query the current value of the specified input-channel.
                * @return The current value of the input channel.
                */
                inline input::state getCurrent( void ) const noexcept
                {
                    return current;
                }
                /**
                * @brief Set/Change the pin number for the provided channel.
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
            friend class observer;
        };

        /**
        * @brief The input channel observer class.
        */
        class observer {
            private:
                list nodes;
                qOS::timer waitDebounce;
                qOS::duration_t debounceTime{ 100_ms };
                channelReaderFcn_t channelReader{ nullptr };
                observer( observer const& ) = delete;
                void operator=( observer const& ) = delete;
                inline state readPin( channel * const n )
                {
                    state s;
                    const state vOn = ( n->negValue ) ? state::OFF : state::ON;
                    const state vOff = ( n->negValue ) ? state::ON : state::OFF;

                    if ( nullptr != channelReader ) {
                        s = channelReader( static_cast<uint8_t>( n->xChannel ) ) ? vOn : vOff;
                    }
                    else {
                        s = state::UNKNOWN;
                    }
                    return s;
                }
            public:
                /**
                * @brief Initialize a input observer instance
                * @param[in] rFcn A pointer to a function that reads the specific
                * channel
                * @param[in] timeDebounce The specified time  to bypass the
                * bounce of the digital input channels
                * @return @c true on success. Otherwise @c false.
                */
                observer( const channelReaderFcn_t& rFcn, const qOS::duration_t timeDebounce ) noexcept
                {
                    debounceTime = timeDebounce;
                    channelReader = rFcn;
                }
                /**
                * @brief Add a channel to the observer instance
                * @param[in] n The input-Channel to observe
                * @return @c true on success. Otherwise @c false.
                */
                bool add( channel& n ) noexcept
                {
                    n.prevPinValue = readPin( &n );
                    n.tChange = clock::getTick();
                    return nodes.insert( &n );
                }
                /**
                * @brief Update the state of all channels inside the observer instance
                * (Non-Blocking call).
                * @return @c true when the input channels have been updated after the
                * debounce time. Otherwise @c false.
                */
                bool update( void ) noexcept;
        };

        /** @}*/
    }

    /** @}*/
}

#endif /*QOS_EDGE_CHECK*/
