#ifndef QOS_EDGE_CHECK
#define QOS_EDGE_CHECK

#include "include/types.hpp"
#include "include/clock.hpp"

namespace qOS {

    class edgeCheck;

    namespace ec {

        enum pinState {
            UNKNOWN = -1,
            OFF = 0,
            ON = 1,
            RISING_EDGE = 2,
            FALLING_EDGE = 3,
        };

        class node {
            private:
                node *next{ nullptr };
                void *xPort{ nullptr };
                pinState prevPinValue{ pinState::UNKNOWN  };
                pinState status{ pinState::UNKNOWN };
                index_t xPin{ 0u };
                node( node const& ) = delete;
                void operator=( node const& ) = delete;
            public:
                node() = default;
                inline pinState getStatus( void ) const noexcept
                {
                    return status;
                }
                bool selectPin( const index_t pinNumber ) noexcept;
            friend class qOS::edgeCheck;
        };

        enum reg {
            SIZE_8_BIT,
            SIZE_16_BIT,
            SIZE_32_BIT,
        };
        using nodeReaderFcn_t = bool (*)( void *, index_t );
    }


    class edgeCheck {
        private:
            void stateCheck( void );
            void stateWait( void );
            void stateUpdate( void );
            void (edgeCheck::* state)( void ) = nullptr;
            ec::node *head{ nullptr};
            qOS::clock_t start{ 0u };
            qOS::clock_t debounceTime{ 0u };
            ec::nodeReaderFcn_t reader{ nullptr };
            ec::nodeReaderFcn_t getReader( ec::reg rSize ) noexcept;
            edgeCheck( edgeCheck const& ) = delete;
            void operator=( edgeCheck const& ) = delete;
        public:
            edgeCheck( ec::reg rSize, const qOS::clock_t timeDebounce ) noexcept;
            bool addNode( ec::node& n, void *portAddress, const index_t pinNumber ) noexcept;
            bool update( void ) noexcept;
    };

}
#endif /*QOS_EDGE_CHECK*/
