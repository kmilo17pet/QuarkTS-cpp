#ifndef QOS_CPP_QUEUE
#define QOS_CPP_QUEUE

#include <types.hpp>

namespace qOS {

    enum class queueSendMode {
        TO_BACK,
        TO_FRONT
    };

    class queue {
        private:
            std::uint8_t *head{ nullptr };
            std::uint8_t *tail{ nullptr };
            std::uint8_t *writer{ nullptr };
            std::uint8_t *reader{ nullptr };
            volatile std::size_t itemsWaiting = 0u;
            std::size_t itemsCount = 0u;
            std::size_t itemSize = 0u;
            void moveReader( void ) noexcept;
            void copyDataFromQueue( void * const dst ) noexcept;
            void copyDataToQueue( const void *itemToQueue, const queueSendMode xPosition ) noexcept; 
            queue( queue const& ) = delete;
            void operator=( queue const& ) = delete;
        public:
            queue() = default;
            queue( void *pData, const std::size_t size, const std::size_t count ) noexcept;
            bool setup( void *pData, const std::size_t size, const std::size_t count ) noexcept;
            void reset( void ) noexcept;
            bool isEmpty( void ) const noexcept;
            bool isFull( void ) const noexcept;
            std::size_t count( void ) const noexcept;
            std::size_t itemsAvailable( void ) const noexcept;
            bool removeFront( void ) noexcept;
            bool receive( void *dst ) noexcept;
            bool send( void *itemToQueue, const queueSendMode pos ) noexcept;
            inline bool send( void *itemToQueue ) noexcept
            {
                return send( itemToQueue, queueSendMode::TO_BACK );
            }
            void* peek( void ) const noexcept;
            bool isReady( void ) const noexcept;
            std::size_t getItemSize( void ) noexcept;
    };
}

#endif /*QOS_CPP_QUEUE*/
