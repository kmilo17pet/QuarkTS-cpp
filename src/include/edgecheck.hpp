#ifndef QOS_EDGE_CHECK
#define QOS_EDGE_CHECK

#include "include/types.hpp"
#include "include/list.hpp"
#include "include/clock.hpp"

namespace qOS {


    /** @addtogroup qedgecheck I/O Groups for edge checking
    * @brief An interface to manage and simplify the value(with edge-checking)
    * of incoming digital-signal groups.
    *  @{
    */

    class edgeCheck;
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

        /**
        * @brief An input node object for edge checking.
        */
        class inNode : protected node {
            private:
                void *xPort{ nullptr };
                pinState prevPinValue{ pinState::UNKNOWN  };
                pinState status{ pinState::UNKNOWN };
                index_t xPin{ 0u };
                inNode( inNode const& ) = delete;
                void operator=( inNode const& ) = delete;
            public:
                inNode() = default;
                /**
                * @brief Query the status of the specified input-node.
                * @return The status of the input node.
                */
                inline pinState getStatus( void ) const noexcept
                {
                    return status;
                }
                /**
                * @brief Set/Change the pin number for the provided node.
                * @param[in] pinNumber The specified Pin to read from PortAddress.
                * @return @c true on success. Otherwise @c false.
                */
                bool selectPin( const index_t pinNumber ) noexcept;
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
        /*! @cond  */
        using nodeReaderFcn_t = bool (*)( void *, index_t );
        /*! @endcond  */

        /** @}*/
    }

    /**
    * @brief An I/O edge check object.
    */
    class edgeCheck {
        private:
            list nodes;
            void stateCheck( void );
            void stateWait( void );
            void stateUpdate( void );
            void (edgeCheck::* state)( void ) = nullptr;
            qOS::clock_t start{ 0u };
            qOS::clock_t debounceTime{ 0u };
            ec::nodeReaderFcn_t reader{ nullptr };
            ec::nodeReaderFcn_t getReader( ec::reg rSize ) noexcept;
            edgeCheck( edgeCheck const& ) = delete;
            void operator=( edgeCheck const& ) = delete;
        public:
            /**
            * @brief Initialize a I/O Edge-Check instance
            * @param[in] rSize The specific-core register size: ec::reg::SIZE_8_BIT, 
            * ec::reg::SIZE_16_BIT or ec::reg::SIZE_32_BIT(Default)
            * @param[in] timeDebounce The specified time given in milliseconds 
            * to bypass the bounce of the input nodes
            * @return @c true on success. Otherwise @c false.
            */
            edgeCheck( ec::reg rSize, const qOS::clock_t timeDebounce ) noexcept;
            /**
            * @brief Add an input node to the Edge-Check instance
            * @param[in] n An input-Node object
            * @param[in] portAddress The address of the core PORTx-register to read the
            * levels of the specified PinNumber
            * @param[in] pinNumber The specified Pin to read from PortAddress
            * @return @c true on success. Otherwise @c false.
            */
            bool add( ec::inNode& n, void *portAddress, const index_t pinNumber ) noexcept;
            /**
            * @brief Update the status of all nodes inside the Input Edge-Check instance
            * (Non-Blocking call).
            * @return @c true on success. Otherwise @c false.
            */
            bool update( void ) noexcept;
    };

    /** @}*/
}

#endif /*QOS_EDGE_CHECK*/
