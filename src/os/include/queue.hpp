#pragma once

#include <types.hpp>

namespace qOS {

    typedef enum {
        TO_BACK,
        TO_FRONT
    } queueSendMode;

    class queue {
        protected:
            uint8_t *head{ nullptr };
            uint8_t *tail{ nullptr };
            uint8_t *writer{ nullptr };
            uint8_t *reader{ nullptr };
            volatile size_t itemsWaiting = 0u;
            size_t itemsCount = 0u;
            size_t itemSize = 0u;
            void moveReader( void );
            void copyDataFromQueue( void * const dst );
            void copyDataToQueue( const void *itemToQueue, const queueSendMode xPosition ); 
            queue( queue const& ) = delete;      /* not copyable*/
            void operator=( queue const& ) = delete;  /* not assignable*/
        public:
            queue() = default;
            queue( void *pData, const size_t itemSize, const size_t itemsCount );
            bool setup( void *pData, const size_t itemSize, const size_t itemsCount );
            void reset( void );
            bool isEmpty( void ) const;
            bool isFull( void ) const;
            size_t count( void ) const;
            size_t itemsAvailable( void ) const;
            bool removeFront( void );
            bool receive( void *dst );
            bool send( void *itemToQueue, const queueSendMode pos );
            bool send( void *itemToQueue )
            {
                return send( itemToQueue, queueSendMode::TO_BACK );
            }
            void* peek( void ) const;
            bool isReady( void ) const;
            size_t getItemSize( void );
    };
}