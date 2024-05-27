#ifndef QOS_EDGE_CHECK
#define QOS_EDGE_CHECK

#include "include/types.hpp"
#include "include/list.hpp"
#include "include/timer.hpp"
#include "include/helper.hpp"

namespace qOS {


    /** @addtogroup qinput Input channel utilities
    * @brief An interface to manage and simplify the value state (with edge-checking)
    * of incoming binary signals.
    *  @{
    */

    /**
    * @brief Input Interfaces.
    */
    namespace input {

        /*! @cond  */
        class watcher;
        class channel;
        /*! @endcond  */

        /** @addtogroup qinput
        *  @{
        */

        /**
        * @brief An enum with all the possible events that can be detected by the
        * watcher class for a specified input-channel.
        */
        enum class event {
            EXCEPTION = -1,  /**< Error due a bad reading or channel configuration .*/
            FALLING_EDGE,    /**< Event on falling-edge of the input-channel (On analog when the reading is below the rise threshold)*/
            RISING_EDGE,     /**< Event on rising-edge of the digital input-channel(On analog when the reading is above the rise threshold)*/
            ON_CHANGE,       /**< Event on any input-channel change when crossing the thresholds*/
            IN_BAND,         /**< Event when the analog input-channel enters the band*/
            STEADY_IN_HIGH,  /**< Event when the input-channel has been kept on high (or above the rise threshold) for the specified time .*/
            STEADY_IN_LOW,   /**< Event when the input-channel has been kept on low (or below the fall threshold) for the specified time .*/
            STEADY_IN_BAND,  /**< Event when the the analog input-channel has remained within the band for the specified time .*/
            /*! @cond  */
            MAX_EVENTS,
            /*! @endcond  */
        };

        enum class type {
            DIGITAL,            /**< Digital input channel.*/
            ANALOG,             /**< Analog input channel.*/
        };

        /**
        * @brief A pointer to the  wrapper function that reads the specific
        * input-channel
        */
        using channelReaderFcn_t = int (*)( uint8_t );
        /**
        * @brief A pointer to the input-channel event callback
        */
        using eventCallback_t = void(*)( channel&, const event );

        /*! @cond  */
        using channelStateFcn_t = void(*)( channel&, int );
        /*! @endcond  */

        /**
        * @brief An digital input-channel object.
        */
        class channel : protected node {
            private:
                bool negate{ false };
                type channelType;
                eventCallback_t cb[ static_cast<size_t>( input::event::MAX_EVENTS ) ] = { nullptr }; // skipcq: CXX-W2066
                qOS::clock_t tChange{ 0U };
                qOS::clock_t tSteadyOn{ 0xFFFFFFFFU };
                qOS::clock_t tSteadyOff{ 0xFFFFFFFFU };
                qOS::clock_t tSteadyBand{ 0xFFFFFFFFU };
                uint8_t xChannel;
                int riseThreshold;
                int fallThreshold;
                int hysteresis{ 20 };
                void *userData{ nullptr };
                channel( channel const& ) = delete;
                void operator=( channel const& ) = delete;
                bool registerEvent( const event e, const eventCallback_t &c, const qOS::duration_t t = 1_sec ) noexcept;
                inline bool unregisterEvent( event e ) noexcept
                {
                    return registerEvent( e, nullptr );
                }
                inline void invokeEvent( input::event e )
                {
                    const auto eventIndex = static_cast<int>( e );
                    if ( nullptr != cb[ eventIndex ] ) {
                        cb[ eventIndex ]( *this, e );
                    }
                }
                inline void cbInit( void )
                {
                    for ( int i = 0 ; i < static_cast<int>( input::event::MAX_EVENTS ) ; ++i ) {
                        cb[ i ] = nullptr;
                    }
                }
                channelStateFcn_t channelState{ nullptr };
                static void digitalFallingEdgeState( channel& c, int value );
                static void digitalRisingEdgeState( channel& c, int value );
                static void digitalSteadyInHighState( channel& c, int value );
                static void digitalSteadyInLowState( channel& c, int value );

                static void analogFallingEdgeState( channel& c, int value );
                static void analogRisingEdgeState( channel& c, int value );
                static void analogInBandState( channel& c, int value );
                static void analogSteadyInHighState( channel& c, int value );
                static void analogSteadyInLowState( channel& c, int value );
                static void analogSteadyInBandState( channel& c, int value );
            public:
                virtual ~channel() {}
                /**
                * @brief Constructor for the analog input channel instance.
                * @note Both lower and upper define the band
                * @param[in] inputChannel The specified channel(pin) to read.
                * @param[in] lowerThreshold The lower threshold value.
                * @param[in] upperThreshold The upper threshold value.
                * @param[in] h Hysteresis for the in-band transition.
                */
                channel( const uint8_t inputChannel, const int lowerThreshold, const int upperThreshold, const int h = 20 ) : xChannel( inputChannel ), riseThreshold( upperThreshold ), fallThreshold( lowerThreshold ), hysteresis( h )
                {
                    channelType = input::type::ANALOG;
                    cbInit();
                }
                /**
                * @brief Constructor for the digital input channel instance.
                * @param[in] inputChannel The specified channel(pin) to read.
                * @param[in] invert To invert/negate the raw-reading.
                */
                channel( uint8_t inputChannel, bool invert = false ) : negate( invert), xChannel( inputChannel )
                {
                    channelType = input::type::DIGITAL;
                    cbInit();

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
                /**
                * @brief Get the channel(pin) number.
                * @return The channel(pin) number.
                */
                inline uint8_t getChannel( void ) const
                {
                    return xChannel;
                }
                /**
                * @brief Get the channel type.
                * @return The channel type.
                */
                inline input::type getChannelType( void ) const
                {
                    return channelType;
                }

                /**
                * @brief Set the channel user-data.
                * @param[in] A pointer to the user-data
                */
                inline void setUserData( void* pUserData )
                {
                    userData = pUserData;
                }
                /**
                * @brief Get the channel user-data.
                * @return A pointer to the user-data
                */
                inline void* getUserData( void )
                {
                    return userData;
                }
            friend class watcher;
        };

        /**
        * @brief The digital input-channel watcher class.
        */
        class watcher : protected node {
            private:
                eventCallback_t exception{ nullptr };
                list nodes;
                qOS::timer waitDebounce;
                qOS::duration_t debounceTime{ 100_ms };
                channelReaderFcn_t digitalReader{ nullptr };
                channelReaderFcn_t analogReader{ nullptr };
                watcher( watcher const& ) = delete;
                void operator=( watcher const& ) = delete;
                void watchDigital( channel& c ) noexcept;
                void watchAnalog( channel& c ) noexcept;
                inline void exceptionEvent( channel& c )
                {
                    if ( nullptr != exception ) {
                        exception( c, input::event::EXCEPTION );
                    }
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
                watcher( const channelReaderFcn_t& rDigital, const channelReaderFcn_t& rAnalog, const qOS::duration_t timeDebounce ) :
                    debounceTime( timeDebounce ), digitalReader( rDigital ), analogReader( rAnalog ) {}
                /**
                * @brief Add a channel to the watcher instance
                * @param[in] n The input-Channel to watch
                * @return @c true on success. Otherwise @c false.
                */
                bool add( channel& n ) noexcept;
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
                * @param[in] t The time associated to the event (only valid for
                * certain events).
                * @return @c true on success. Otherwise @c false.
                */
                bool registerEvent( channel& n, const event e, const eventCallback_t &c, const qOS::duration_t t = 1_sec ) noexcept
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
                bool unregisterEvent( channel& n, const event e ) noexcept
                {
                    return n.unregisterEvent( e );
                }
                /**
                * @brief Register an event-callback to be activated in the
                * watcher instance for all the input-channels being added.
                * @param[in] e The event to register.
                * @param[in] c The callback to be invoked when the event is detected.
                * @param[in] t The time associated to the event (only valid for
                * certain events).
                * @return @c true if operation succeeds in all input-channels.
                * Otherwise @c false.
                */
                bool registerEvent( const event e, const eventCallback_t &c, const qOS::duration_t t = 1_sec ) noexcept;
                /**
                * @brief Un-Register an event-callback for all input-channels
                * being added.
                * @param[in] e The event to unregister.
                * @return @c true if operation succeeds in all input-channels.
                * Otherwise @c false.
                */
                bool unregisterEvent( event e ) noexcept
                {
                    return registerEvent( e, nullptr );
                }
        };

        /** @}*/
    }

    /** @}*/
}

#endif /*QOS_EDGE_CHECK*/
