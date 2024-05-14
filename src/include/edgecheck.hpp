#ifndef QOS_EDGE_CHECK
#define QOS_EDGE_CHECK

#include "include/types.hpp"
#include "include/list.hpp"
#include "include/timer.hpp"

namespace qOS {


    /** @addtogroup qedgecheck I/O Groups for edge checking
    * @brief An interface to manage and simplify the value(with edge-checking)
    * of incoming digital-signal groups.
    *  @{
    */

    class edgeCheck;

    /**
    * @brief Edge-Check interfaces for I/O groups.
    */
    namespace ec {

        /** @addtogroup qedgecheck
        *  @{
        */

        /**
        * @brief An enum with all the possible states detected by the
        * qOS::edgeCheck class  for a specified input node.
        */
        enum class pinState {
            UNKNOWN = -1,
            OFF = 0,
            ON = 1,
            RISING_EDGE = 2,
            FALLING_EDGE = 3,
        };

        /*! @cond  */
        enum class inNodeEvent {
            IN_NODE_FALLING,
            IN_NODE_RISING,
            IN_NODE_PRESSED,
            IN_NODE_RELEASED,
        };
        using nodePortReaderFcn_t = bool (*)( void *, index_t );
        using nodePinReaderFcn_t = int (*)( uint8_t );
        using inNodeCallback_t = void(*)( uint8_t, const inNodeEvent );
        /*! @endcond  */

        /**
        * @brief An input node object for edge checking.
        */
        class inNode : protected node {
            private:
                void *xPort;
                ec::inNodeCallback_t fallingCB;
                ec::inNodeCallback_t risingCB;
                ec::inNodeCallback_t pressedCB;
                ec::inNodeCallback_t releasedCB;
                bool bPressed{ false };
                bool bReleased{ false };
                pinState prevPinValue{ pinState::UNKNOWN  };
                pinState state{ pinState::UNKNOWN };
                pinState current{ pinState::UNKNOWN };
                qOS::clock_t tChange;
                qOS::clock_t tPressed;
                qOS::clock_t tReleased;
                bool negValue{ false };
                uint8_t xPin;
                inNode( inNode const& ) = delete;
                void operator=( inNode const& ) = delete;
            public:
                inNode( uint8_t inputPin, bool invert = false, void *portAddress = nullptr ) : xPort( portAddress ), negValue( invert ), xPin( inputPin ) {}
                virtual ~inNode() {}
                bool setCallback( ec::inNodeEvent e, ec::inNodeCallback_t c, qOS::clock_t t = 1000U );
                /**
                * @brief Query the state of the specified input-node.
                * @return The state of the input node.
                */
                inline pinState getState( void ) const noexcept
                {
                    return state;
                }
                /**
                * @brief Query the current value of the specified input-node.
                * @return The current value of the input node.
                */
                inline pinState getCurrent( void ) const noexcept
                {
                    return current;
                }
                /**
                * @brief Set/Change the pin number for the provided node.
                * @param[in] pinNumber The specified Pin to read from PortAddress.
                * @return @c true on success. Otherwise @c false.
                */
                inline bool selectPin( const uint8_t pinNumber ) noexcept
                {
                    bool retValue = false;

                    if ( pinNumber < 32U ) {
                        xPin = pinNumber;
                        retValue = true;
                    }

                    return retValue;
                }
            friend class qOS::edgeCheck;
        };

        /**
        * @brief An enum class to specify the input register size for edge checking.
        */
        enum class reg {
            SIZE_8_BIT,
            SIZE_16_BIT,
            SIZE_32_BIT,
        };



        /** @}*/
    }

    /**
    * @brief An I/O edge check object.
    */
    class edgeCheck {
        private:
            list nodes;
            qOS::timer waitDebounce;
            qOS::duration_t debounceTime{ 100_ms };
            ec::nodePortReaderFcn_t portReader{ nullptr };
            ec::nodePinReaderFcn_t pinReader{ nullptr };
            ec::nodePortReaderFcn_t getReader( ec::reg rSize ) noexcept;
            edgeCheck( edgeCheck const& ) = delete;
            void operator=( edgeCheck const& ) = delete;
            inline ec::pinState readPin( ec::inNode * const n )
            {
                ec::pinState s;
                const ec::pinState vOn = ( n->negValue ) ? ec::pinState::OFF : ec::pinState::ON;
                const ec::pinState vOff = ( n->negValue ) ? ec::pinState::ON : ec::pinState::OFF;

                if ( nullptr != pinReader ) {
                    /*cstat -MISRAC++2008-5-0-14*/
                    s = pinReader( static_cast<uint8_t>( n->xPin ) ) ? vOn : vOff;
                }
                else if ( nullptr != portReader ) {
                    s = ( portReader( n->xPort, n->xPin ) ) ? vOn : vOff;
                    /*cstat +MISRAC++2008-5-0-14*/
                }
                else {
                    s = ec::pinState::UNKNOWN;
                }
                return s;
            }
        public:
            /**
            * @brief Initialize a I/O Edge-Check instance
            * @param[in] pinReader A pointer to a function that reads the specific
            * pin
            * @param[in] timeDebounce The specified time given in milliseconds
            * to bypass the bounce of the input nodes
            * @return @c true on success. Otherwise @c false.
            */
            edgeCheck( ec::nodePinReaderFcn_t bitReader, const qOS::duration_t timeDebounce ) noexcept
            {
                debounceTime = timeDebounce;
                pinReader = bitReader;
            }
            /**
            * @brief Initialize a I/O Edge-Check instance
            * @param[in] rSize The specific-core register size: ec::reg::SIZE_8_BIT,
            * ec::reg::SIZE_16_BIT or ec::reg::SIZE_32_BIT(Default)
            * @param[in] timeDebounce The specified time given in milliseconds
            * to bypass the bounce of the input nodes
            * @return @c true on success. Otherwise @c false.
            */
            edgeCheck( ec::reg rSize, const qOS::duration_t timeDebounce ) noexcept
            {
                debounceTime = timeDebounce;
                portReader = getReader( rSize );
            }
            /**
            * @brief Add an input node to the Edge-Check instance
            * @param[in] n An input-Node object
            * @param[in] portAddress The address of the core PORTx-register to read the
            * levels of the specified PinNumber
            * @param[in] pinNumber The specified Pin to read from PortAddress
            * @return @c true on success. Otherwise @c false.
            */
            bool add( ec::inNode& n ) noexcept
            {
                n.prevPinValue = readPin( &n );
                n.tChange = clock::getTick();
                return nodes.insert( &n );
            }
            /**
            * @brief Update the status of all nodes inside the Input Edge-Check instance
            * (Non-Blocking call).
            * @return @c true when the input nodes have been updated after the
            * debounce time. Otherwise @c false.
            */
            bool update( void ) noexcept;
    };

    /** @}*/
}

#endif /*QOS_EDGE_CHECK*/
