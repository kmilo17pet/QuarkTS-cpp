#ifndef QOS_EDGE_CHECK
#define QOS_EDGE_CHECK

#include "include/types.hpp"
#include "include/list.hpp"
#include "include/timer.hpp"
#include "include/helper.hpp"

namespace qOS {


    /** @addtogroup qinput Input channel events
    * @brief A comprehensive event class for efficient, maintainable working with
    * input channels.
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
            NONE = 0,
            EXCEPTION,              /**< Error due a bad reading or channel configuration .*/
            ON_CHANGE,              /**< Event on any input-channel change when crossing the thresholds*/
            FALLING_EDGE,           /**< Event on falling-edge( high to low ) of the digital input-channel*/
            RISING_EDGE,            /**< Event on rising-edge( low to high ) of the digital input-channel*/
            PULSATION_DOUBLE,       /**< Event when the digital input is pulsated two times within the interval*/
            PULSATION_TRIPLE,       /**< Event when the digital input is pulsated three times within the interval*/
            PULSATION_MULTI,        /**< Event when the digital input is pulsated more than three times within the interval*/
            HIGH_THRESHOLD,         /**< Event when the analog input-channel reading is above the high threshold*/
            LOW_THRESHOLD,          /**< Event when the analog input-channel reading is below the low threshold*/
            IN_BAND,                /**< Event when the analog input-channel reading enters the band defined by the low-high thresholds*/
            STEADY_IN_HIGH,         /**< Event when the input-channel has been kept on high (or above the high threshold) for the specified time .*/
            STEADY_IN_LOW,          /**< Event when the input-channel has been kept on low (or below the low threshold) for the specified time .*/
            STEADY_IN_BAND,         /**< Event when the analog input-channel has remained within the band for the specified time .*/
            DELTA,                  /**< Event when the difference of the last and latest reading of an analog input channel is greater than the defined delta*/
            STEP_UP,                /**< Event on step reading of the analog-input channel*/
            STEP_DOWN,              /**< Event on step reading of the analog-input channel*/
            /*! @cond  */
            MAX_EVENTS,
            STEP = STEP_UP
            /*! @endcond  */
        };

        /**
        * @brief An enum class to define the types of input channels
        */
        enum class type {
            DIGITAL_CHANNEL,            /**< Digital input channel.*/
            ANALOG_CHANNEL,             /**< Analog input channel.*/
        };


        using digitalValue_t = int;
        using analogValue_t = uint32_t;

        /**
        * @brief A pointer to the  wrapper function that reads the specific
        * digital input-channel
        *
        * Prototype: @code digitalValue_t readerFcn( uint8_t channelNumber ) @endcode
        */
        using digitalReaderFcn_t = digitalValue_t (*)( uint8_t );

        /**
        * @brief A pointer to the  wrapper function that reads the specific
        * analog input-channel
        *
        * Prototype: @code analogValue_t readerFcn( uint8_t channelNumber ) @endcode
        */
        using analogReaderFcn_t = analogValue_t (*)( uint8_t );



        /**
        * @brief A pointer to the input-channel event callback
        *
        * Prototype: @code void xCallback( input::channel& c ) @endcode
        */
        using eventCallback_t = void(*)( channel& );

        class channel : protected node {
            /*! @cond  */
            protected:
                event lastEvent{ event::NONE };
                uint8_t number;
                void *userData{ nullptr };
                eventCallback_t callback{ nullptr };
                qOS::clock_t tChange{ 0U };
                qOS::clock_t tSteadyHigh{ 0xFFFFFFFFU };
                qOS::clock_t tSteadyLow{ 0xFFFFFFFFU };
                virtual void updateReading( bool act ) noexcept = 0;
                virtual void evaluateState( void ) noexcept = 0;
                virtual bool isValidConfig( void ) const noexcept = 0;
                virtual void setInitalState( void ) noexcept = 0;
                inline void dispatchEvent( event e ) noexcept
                {
                    lastEvent = e;
                    callback( *this );
                }
            /*! @endcond  */
            public:
                /*! @cond  */
                virtual ~channel() {}
                channel( uint8_t channelNumber ) noexcept : number( channelNumber ) {}
                /*! @endcond  */

                /**
                * @brief Get the channel type.
                * @return The channel type.
                */
                virtual type getType( void ) const noexcept = 0;
                /**
                * @brief Retrieves the last event for the given input channel.
                * @return @c The last input-channel event.
                */
                inline event getEvent( void ) const noexcept
                {
                    return lastEvent;
                }
                /**
                * @brief Set the callback function when event are detected on the
                * input input channel.
                * @param[in] cb The callback function
                */
                inline bool setCallback( const eventCallback_t cb ) noexcept
                {
                    callback = cb;
                    return ( cb != callback );
                }
                /**
                * @brief Set/Change the channel(pin) number.
                * @param[in] inputChannel The specified channel number (pin) to read.
                * @return @c true on success. Otherwise @c false.
                */
                inline bool setChannel( const uint8_t inputChannel ) noexcept
                {
                    bool retValue = false;

                    if ( inputChannel < 32U ) {
                        number = inputChannel;
                        retValue = true;
                    }

                    return retValue;
                }
                /**
                * @brief Get the channel(pin) number.
                * @return The channel(pin) number.
                */
                inline uint8_t getChannel( void ) const noexcept
                {
                    return number;
                }
                /**
                * @brief Set the channel user-data.
                * @param[in] pUserData A pointer to the user-data
                */
                inline void setUserData( void* pUserData ) noexcept
                {
                    userData = pUserData;
                }
                /**
                * @brief Get the channel user-data.
                * @return A pointer to the user-data
                */
                inline void* getUserData( void ) noexcept
                {
                    return userData;
                }
                /**
                * @brief Check if the channel value is shared with other channel
                * with the same (pin) number.
                * @return @c true if shared. Otherwise @c false.
                */
                virtual bool isShared( void ) const noexcept = 0;
                /**
                * @brief Set the timeout for the specified event.
                * @param[in] e The event where the timeout will be set.
                * @param[in] t The value of the timeout.
                * @return @c true on success. Otherwise @c false.
                */
                virtual bool setTime( const event e, const qOS::duration_t t ) noexcept = 0;
                /**
                * @brief Set the parameter for the specified event.
                * @param[in] e The event where the parameter will be set.
                * @param[in] p The value of the parameter.
                * @return @c true on success. Otherwise @c false.
                */
                virtual bool setParameter( const event e, const analogValue_t p ) noexcept = 0;
                /**
                * @brief Get pulsation count for the digital input.
                * @note No valid on analog inputs
                * @return The current pulsation count.
                */
                virtual uint8_t getCount( void ) const noexcept = 0;
                /*! @cond  */
                virtual bool setReader( digitalReaderFcn_t r ) noexcept = 0;
                virtual bool setReader( analogReaderFcn_t r ) noexcept = 0;
                /*! @endcond  */

                virtual bool unShare( void ) noexcept = 0;
                friend class watcher;
        };


        /**
        * @brief A digital input-channel object.
        */
        class digitalChannel : public channel {
            using channelStateFcn_t = void(*)( digitalChannel& );
            private:
                digitalValue_t value{ 0 };
                digitalValue_t *ptrValue{ &value };
                digitalReaderFcn_t reader{ nullptr };
                channelStateFcn_t channelState{ nullptr };
                bool negate{ false };
                qOS::clock_t pulsationInterval{ 250U };
                uint8_t pulsationCount{ 0 };
                void updateReading( bool act ) noexcept override;
                void setInitalState( void ) noexcept override;
                bool isValidConfig( void ) const noexcept override
                {
                    return true;
                }
                void evaluateState( void ) noexcept override
                {
                    channelState( *this );
                }
                static void fallingEdgeState( digitalChannel& c );
                static void risingEdgeState( digitalChannel& c );
                static void steadyInHighState( digitalChannel& c );
                static void steadyInLowState( digitalChannel& c );
                digitalChannel( digitalChannel const& ) = delete;
                void operator=( digitalChannel const& ) = delete;
            public:
                /*! @cond  */
                virtual ~digitalChannel() {}
                /*! @endcond  */
                /**
                * @brief Constructor for the digital input channel instance.
                * @param[in] inputChannel The specified channel(pin) number to read.
                * @param[in] invert To invert/negate the raw-reading.
                */
                explicit digitalChannel( const uint8_t inputChannel, bool invert = false ) noexcept : channel( inputChannel ), negate( invert) {}
                /**
                * @brief Get the channel type.
                * @return The channel type.
                */
                type getType( void ) const noexcept override
                {
                    return type::DIGITAL_CHANNEL;
                }
                /**
                * @brief Set the timeout for the specified event.
                * @param[in] e The event where the timeout will be set.
                * @param[in] t The value of the timeout.
                * @return @c true on success. Otherwise @c false.
                */
                bool setTime( const event e, const qOS::duration_t t ) noexcept override;
                /**
                * @brief Set the parameter for the specified event.
                * @param[in] e The event where the parameter will be set.
                * @param[in] p The value of the parameter.
                * @return @c true on success. Otherwise @c false.
                */
                bool setParameter( const event e, const analogValue_t p ) noexcept override;
                /**
                * @brief Get pulsation count for the digital input.
                * @note No valid on analog inputs
                * @return The current pulsation count.
                */
                uint8_t getCount( void ) const noexcept override
                {
                    return pulsationCount;
                }
                /**
                * @brief Check if the channel value is shared with other channel
                * with the same (pin) number.
                * @return @c true if shared. Otherwise @c false.
                */
                bool isShared( void ) const noexcept override
                {
                    return ( &value != ptrValue );
                }
                /**
                * @brief Assign the function that is in charge of reading the
                * specific digital input.
                * @param[in] r The channel reader function.
                * @return @c true on success. Otherwise @c false.
                */
                bool setReader( digitalReaderFcn_t r ) noexcept override
                {
                    reader = r;
                    return true;
                }
                /*! @cond  */
                bool setReader( analogReaderFcn_t r ) noexcept override
                {
                    (void)r;
                    return false;
                }
                bool unShare( void ) noexcept override
                {
                    ptrValue = &value;
                    return true;
                }
                /*! @endcond  */
            friend class watcher;
        };

        /**
        * @brief An analog input-channel object.
        */
        class analogChannel : public channel {
            using channelStateFcn_t = void(*)( analogChannel& );
            private:
                analogValue_t value{ 0 };
                analogValue_t *ptrValue{ &value };
                analogReaderFcn_t reader{ nullptr };
                channelStateFcn_t channelState{ nullptr };
                analogValue_t high{ 800U };
                analogValue_t low{ 200U };
                analogValue_t lastStep{ 0U };
                analogValue_t lastSampled{ 0U };
                analogValue_t delta{ 0xFFFFFFFFU };
                analogValue_t step{ 0xFFFFFFFFU };
                analogValue_t hysteresis{ 20U };
                qOS::clock_t tSteadyBand{ 0xFFFFFFFFU };

                void updateReading( bool act ) noexcept override;
                void setInitalState( void ) noexcept override;
                bool isValidConfig( void ) const noexcept override
                {
                    return ( high - low ) > hysteresis;
                }
                void evaluateState( void ) noexcept override
                {
                    channelState( *this );
                }
                static void lowThresholdState( analogChannel& c );
                static void highThresholdState( analogChannel& c );
                static void inBandState( analogChannel& c );
                static void steadyInHighState( analogChannel& c );
                static void steadyInLowState( analogChannel& c );
                static void steadyInBandState( analogChannel& c );
                analogChannel( analogChannel const& ) = delete;
                void operator=( analogChannel const& ) = delete;
            public:
                /*! @cond  */
                virtual ~analogChannel() {}
                /*! @endcond  */
                /**
                * @brief Constructor for the analog input channel instance.
                * @note Both lower and upper define the band
                * @param[in] inputChannel The specified channel(pin) number to read.
                * @param[in] lowerThreshold The lower threshold value.
                * @param[in] upperThreshold The upper threshold value.
                * @param[in] h Hysteresis for the in-band transition.
                */
                analogChannel( const uint8_t inputChannel, const analogValue_t lowerThreshold, const analogValue_t upperThreshold, const analogValue_t h = 20 ) noexcept
                : channel( inputChannel ),
                  high( upperThreshold ),
                  low( lowerThreshold ),
                  hysteresis( h ) {}
                /**
                * @brief Get the channel type.
                * @return The channel type.
                */
                type getType( void ) const noexcept override
                {
                    return type::ANALOG_CHANNEL;
                }
                /**
                * @brief Set the timeout for the specified event.
                * @param[in] e The event where the timeout will be set.
                * @param[in] t The value of the timeout.
                * @return @c true on success. Otherwise @c false.
                */
                bool setTime( const event e, const qOS::duration_t t ) noexcept override;
                /**
                * @brief Set the parameter for the specified event.
                * @param[in] e The event where the parameter will be set.
                * @param[in] p The value of the parameter.
                * @return @c true on success. Otherwise @c false.
                */
                bool setParameter( const event e, const analogValue_t p ) noexcept override;
                /**
                * @brief Get pulsation count for the digital input.
                * @note No valid on analog inputs
                * @return The current pulsation count.
                */
                uint8_t getCount( void ) const noexcept override
                {
                    return 0;
                }
                /**
                * @brief Check if the channel value is shared with other channel
                * with the same (pin) number.
                * @return @c true if shared. Otherwise @c false.
                */
                bool isShared( void ) const noexcept override
                {
                    return ( &value != ptrValue );
                }
                /*! @cond  */
                bool setReader( digitalReaderFcn_t r ) noexcept override
                {
                    (void)r;
                    return false;
                }
                /*! @endcond  */
                /**
                * @brief Assign the function that is in charge of reading the
                * specific analog input.
                * @param[in] r The channel reader function.
                * @return @c true on success. Otherwise @c false.
                */
                bool setReader( analogReaderFcn_t r ) noexcept override
                {
                    reader = r;
                    return true;
                }
                /**
                * @brief Unshares the specified input channel if was marked as
                * shared.
                * @return @c true on success. Otherwise @c false.
                */
                bool unShare( void ) noexcept override
                {
                    ptrValue = &value;
                    return true;
                }
            friend class watcher;
        };

        /*cstat -MISRAC++2008-2-13-3*/
        /**
        * @brief The digital input-channel watcher class.
        */
        class watcher : private nonCopyable {
            private:
                eventCallback_t exception{ nullptr };
                list digitalChannels;
                list analogChannels;
                qOS::timer waitDebounce;
                qOS::duration_t debounceTime{ 100_ms };
                digitalReaderFcn_t digitalReader{ nullptr };
                analogReaderFcn_t analogReader{ nullptr };
            public:
                /*! @cond  */
                virtual ~watcher() {}
                /*! @endcond  */
                /**
                * @brief Constructor for the input-watcher instance
                * @param[in] dt The specified time to bypass the bounce of the
                * digital input channels
                */
                explicit watcher( const qOS::duration_t dt = 100_ms ) : debounceTime( dt ) {}
                /**
                * @brief Constructor for the input-watcher instance
                * @param[in] rDigital A pointer to a function that reads the specific
                * digital input channel
                * @param[in] rAnalog A pointer to a function that reads the specific
                * analog input channel
                * @param[in] dt The specified time to bypass the bounce of the
                * digital input channels. Is also used as sample time
                * on analog input channels to trigger input::event::STEP and
                * input::event::DELTA events.
                */
                watcher( const digitalReaderFcn_t& rDigital, const analogReaderFcn_t& rAnalog, const qOS::duration_t dt = 100_ms ) :
                    debounceTime( dt ), digitalReader( rDigital ), analogReader( rAnalog ) {}
                /**
                * @brief Add a channel to the watcher instance
                * @param[in] c The input-Channel to watch
                * @return @c true on success. Otherwise @c false.
                */
                bool add( channel& c ) noexcept;
                /**
                * @brief Add a channel to the watcher instance
                * @param[in] c The input-Channel to watch
                * @param[in] cb The callback function for the input-channel
                * @return @c true on success. Otherwise @c false.
                */
                inline bool add( channel& c, eventCallback_t cb ) noexcept
                {
                    (void)c.setCallback( cb );
                    return add( c );
                }
                /**
                * @brief Add a channel to the watcher instance
                * @param[in] c The input-Channel to watch
                * @param[in] fcn The reader function for the digital channel
                * @param[in] cb The callback function for the input-channel
                * @return @c true on success. Otherwise @c false.
                */
                inline bool add( channel&c, digitalReaderFcn_t fcn, eventCallback_t cb ) noexcept
                {
                    bool retValue;
                    (void)c.setCallback( cb );
                    retValue = add( c );
                    (void)c.setReader( fcn );
                    return retValue;
                }
                /**
                * @brief Add a channel to the watcher instance
                * @param[in] c The input-Channel to watch
                * @param[in] fcn The reader function for the analog channel
                * @param[in] cb The callback function for the input-channel
                * @return @c true on success. Otherwise @c false.
                */
                inline bool add( channel&c, analogReaderFcn_t fcn, eventCallback_t cb ) noexcept
                {
                    bool retValue;
                    (void)c.setCallback( cb );
                    retValue = add( c );
                    (void)c.setReader( fcn );
                    return retValue;
                }
                /**
                * @brief Remove a channel to the watcher instance
                * @param[in] c The input-Channel to watch
                * @return @c true on success. Otherwise @c false.
                */
                bool remove( channel& c ) noexcept;
                /**
                * @brief Watch for the state and events for all channels
                * registered inside the watcher instance (Non-Blocking call).
                * @return @c true when the input channels have been completed an
                * updated cycle. Otherwise @c false.
                */
                bool watch( void ) noexcept;
                /**
                * @brief Watch for the state and events for all channels
                * registered inside the watcher instance (Non-Blocking call).
                * @return @c true when the input channels have been completed an
                * updated cycle. Otherwise @c false.
                */
                inline bool operator()( void )
                {
                    return watch();
                }
                /**
                 * @brief Returns the number of analog channels monitored by
                 * this instance.
                 * @return The number of analog channels managed by the watcher
                 * instance.
                 */
                size_t getAnalogChannelsCount( void ) const noexcept;
                /**
                 * @brief Returns the number of digital channels monitored by
                 * this instance.
                 * @return The number of digital channels managed by the watcher
                 * instance.
                 */
                size_t getDigitalChannelsCount( void ) const noexcept;
        };
        /*cstat +MISRAC++2008-2-13-3*/
        /** @}*/
    }

    /** @}*/
}

#endif /*QOS_EDGE_CHECK*/
