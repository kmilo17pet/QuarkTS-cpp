#ifndef QOS_BYTE_BUFFER
#define QOS_BYTE_BUFFER

#include "types.hpp"

namespace qOS {

    class byteBuffer {
        private:
            volatile std::uint8_t *buffer{ nullptr };
            volatile index_t tail{ 0u };
            volatile index_t head{ 0u };
            std::size_t length{ 0u };
            std::size_t checkValidPowerOfTwo( std::size_t k ) noexcept;
            byteBuffer( byteBuffer const& ) = delete;
            void operator=( byteBuffer const& ) = delete;
        public:
            byteBuffer() = default;
            bool setup( volatile std::uint8_t *pBuffer, const size_t length ) noexcept;
            bool put( const std::uint8_t bData ) noexcept;
            bool read( void *dst, const std::size_t n ) noexcept;
            bool get( std::uint8_t *dst ) noexcept;
            std::uint8_t peek( void ) const noexcept;
            bool isEmpty( void ) const noexcept;
            bool isFull( void ) const noexcept;
            std::size_t count( void ) const noexcept;
    };

}
#endif /*QOS_BYTE_BUFFER*/
