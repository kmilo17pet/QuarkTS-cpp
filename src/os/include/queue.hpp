#ifndef QOS_CPP_QUEUE
#define QOS_CPP_QUEUE

#include <types.hpp>

namespace qOS {

    enum class queueSendMode {
        TO_BACK,
        TO_FRONT
    };

    class queue {
        protected:
            std::uint8_t *head{ nullptr };
            std::uint8_t *tail{ nullptr };
            std::uint8_t *writer{ nullptr };
            std::uint8_t *reader{ nullptr };
            volatile std::size_t itemsWaiting = 0u;
            std::size_t itemsCount = 0u;
            std::size_t itemSize = 0u;
            void moveReader( void );
            void copyDataFromQueue( void * const dst );
            void copyDataToQueue( const void *itemToQueue, const queueSendMode xPosition ); 
            queue( queue const& ) = delete;      /* not copyable*/
            void operator=( queue const& ) = delete;  /* not assignable*/
        public:
            queue() = default;
            queue( void *pData, const std::size_t itemSize, const std::size_t itemsCount );
            bool setup( void *pData, const std::size_t itemSize, const std::size_t itemsCount );
            void reset( void );
            bool isEmpty( void ) const;
            bool isFull( void ) const;
            std::size_t count( void ) const;
            std::size_t itemsAvailable( void ) const;
            bool removeFront( void );
            bool receive( void *dst );
            bool send( void *itemToQueue, const queueSendMode pos );
            inline bool send( void *itemToQueue )
            {
                return send( itemToQueue, queueSendMode::TO_BACK );
            }
            void* peek( void ) const;
            bool isReady( void ) const;
            std::size_t getItemSize( void );
    };
}

#endif /*QOS_CPP_QUEUE*/