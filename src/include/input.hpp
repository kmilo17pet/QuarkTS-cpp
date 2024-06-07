#ifndef QOS_EDGE_CHECK
#define QOS_EDGE_CHECK

#include "include/types.hpp"
#include "include/list.hpp"
#include "include/timer.hpp"
#include "include/helper.hpp"

namespace qOS {


    /** @addtogroup qinput Input channel utilities
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
            /*! @cond  */
            MAX_EVENTS
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
        using eventCallback_t = void(*)( channel& );

        class channel : protected node {
            protected:
                int value;
                int *ptrValue{ &value };
                event lastEvent{ event::NONE };
                uint8_t number;
                void *userData{ nullptr };
                eventCallback_t callback{ nullptr };
                qOS::clock_t tChange{ 0U };
                qOS::clock_t tSteadyHigh{ 0xFFFFFFFFU };
                qOS::clock_t tSteadyLow{ 0xFFFFFFFFU };
                virtual void updateReading( channelReaderFcn_t reader ) noexcept = 0;
                virtual void evaluateState( void ) noexcept = 0;
                virtual bool isValidConfig( void ) const noexcept = 0;
                virtual void setInitalState( channelReaderFcn_t reader ) noexcept = 0;
                inline void dispatchEvent( event e ) noexcept
                {
                    lastEvent = e;
                    callback( *this );
                }
            public:
                virtual ~channel() {}
                channel( uint8_t channelNumber ) : number( channelNumber ) {}
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
                * @param[in] inputChannel The specified channel(pin) number to read.
                * @param[in] invert To invert/negate the raw-reading.
                */
                inline bool setCallback( eventCallback_t cb ) noexcept
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
                * @param[in] A pointer to the user-data
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
                inline bool isShared( void ) const noexcept
                {
                    return ( &value != ptrValue );
                }
                /**
                * @brief Set the timeout for the specified event.
                * @param[in] e The event where the timeout will be set.
                * @param[in] t The value of the timeout.
                * @return @c true on success. Otherwise @c false.
                */
                virtual bool setTime( const event e, const qOS::duration_t t ) noexcept = 0;
                /**
                * @brief Set the parameter for the specified event.
                * @param[in] e The event where the timeout will be set.
                * @param[in] p The value of the parameter.
                * @return @c true on success. Otherwise @c false.
                */
                virtual bool setParameter( const event e, const int value ) noexcept = 0;
                /**
                * @brief Get pulsation count for the digital input.
                * @note No valid on analog inputs
                * @return The current pulsation count.
                */
                virtual uint8_t getCount( void ) const noexcept = 0;
                friend class watcher;
        };


        /**
        * @brief A digital input-channel object.
        */
        class digitalChannel : public channel {
            using channelStateFcn_t = void(*)( digitalChannel& );
            private:
                channelStateFcn_t channelState{ nullptr };
                bool negate{ false };
                qOS::clock_t pulsationInterval{ 250U };
                uint8_t pulsationCount{ 0 };
                void updateReading( channelReaderFcn_t reader ) noexcept override;
                void setInitalState( channelReaderFcn_t reader ) noexcept override;
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
                virtual ~digitalChannel() {}
                /**
                * @brief Constructor for the digital input channel instance.
                * @param[in] inputChannel The specified channel(pin) number to read.
                * @param[in] invert To invert/negate the raw-reading.
                */
                digitalChannel( const uint8_t inputChannel, bool invert = false ) : channel( inputChannel ), negate( invert) {}
                /**
                * @brief Get the channel type.
                * @return The channel type.
                */
                type getType( void ) const noexcept override
                {
                    return type::DIGITAL;
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
                * @param[in] e The event where the timeout will be set.
                * @param[in] p The value of the parameter.
                * @return @c true on success. Otherwise @c false.
                */
                bool setParameter( const event e, const int value ) noexcept override;
                /**
                * @brief Get pulsation count for the digital input.
                * @note No valid on analog inputs
                * @return The current pulsation count.
                */
                uint8_t getCount( void ) const noexcept override
                {
                    return pulsationCount;
                }
            friend class watcher;
        };

        /**
        * @brief An analog input-channel object.
        */
        class analogChannel : public channel {
            using channelStateFcn_t = void(*)( analogChannel& );
            private:
                channelStateFcn_t channelState{ nullptr };
                int high{ 800 };
                int low{ 200 };
                int hysteresis{ 20 };
                qOS::clock_t tSteadyBand{ 0xFFFFFFFFU };

                void updateReading( channelReaderFcn_t reader ) noexcept override;
                void setInitalState( channelReaderFcn_t reader ) noexcept override;
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
                virtual ~analogChannel() {}
                /**
                * @brief Constructor for the analog input channel instance.
                * @note Both lower and upper define the band
                * @param[in] inputChannel The specified channel(pin) number to read.
                * @param[in] lowerThreshold The lower threshold value.
                * @param[in] upperThreshold The upper threshold value.
                * @param[in] h Hysteresis for the in-band transition.
                */
                analogChannel( const uint8_t inputChannel, const int lowerThreshold, const int upperThreshold, const int h = 20 ) : channel( inputChannel ), high( upperThreshold ), low( lowerThreshold )
                {
                    hysteresis = ( h < 0 ) ? -h : h;
                }
                /**
                * @brief Get the channel type.
                * @return The channel type.
                */
                type getType( void ) const noexcept override
                {
                    return type::ANALOG;
                }
                /**
                * @brief Set the timeout for the specified event.
                * @param[in] e The event where the timeout will be set.
                * @param[in] t The value of the timeout.
                * @return @c true on success. Otherwise @c false.
                */
                bool setTime( const event e, const qOS::duration_t t ) noexcept = 0;
                /**
                * @brief Set the parameter for the specified event.
                * @param[in] e The event where the timeout will be set.
                * @param[in] p The value of the parameter.
                * @return @c true on success. Otherwise @c false.
                */
                bool setParameter( const event e, const int p ) noexcept override;
                /**
                * @brief Get pulsation count for the digital input.
                * @note No valid on analog inputs
                * @return The current pulsation count.
                */
                uint8_t getCount( void ) const noexcept override
                {
                    return 0;
                }
            friend class watcher;
        };


        /**
        * @brief The digital input-channel watcher class.
        */
        class watcher : protected node {
            private:
                eventCallback_t exception{ nullptr };
                list digitalChannels;
                list analogChannels;
                qOS::timer waitDebounce;
                qOS::duration_t debounceTime{ 100_ms };
                channelReaderFcn_t digitalReader{ nullptr };
                channelReaderFcn_t analogReader{ nullptr };
                watcher( watcher const& ) = delete;
                void operator=( watcher const& ) = delete;
            public:
                virtual ~watcher() {}
                /**
                * @brief Constructor for the input-watcher instance
                * @param[in] rFcn A pointer to a function that reads the specific
                * channel
                * @param[in] timeDebounce The specified time to bypass the
                * bounce of the digital input channels
                * @return @c true on success. Otherwise @c false.
                */
                watcher( const channelReaderFcn_t& rDigital, const channelReaderFcn_t& rAnalog, const qOS::duration_t timeDebounce = 100_ms ) :
                    debounceTime( timeDebounce ), digitalReader( rDigital ), analogReader( rAnalog ) {}
                /**
                * @brief Add a channel to the watcher instance
                * @param[in] c The input-Channel to watch
                * @return @c true on success. Otherwise @c false.
                */
                bool add( channel& c ) noexcept;
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
                /*! @cond  */
                inline void operator()( void )
                {
                    watch();
                }
                /*! @endcond  */
        };
        /** @}*/
    }

    /** @}*/
}

#endif /*QOS_EDGE_CHECK*/
