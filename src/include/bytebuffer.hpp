#ifndef QOS_BYTE_BUFFER
#define QOS_BYTE_BUFFER

#include "include/types.hpp"


namespace qOS {

    /** @addtogroup qbsbuffers Byte-Sized Buffers
     * @brief An interrupt-safe byte-sized ring buffer interface.
     *  @{
     */

    /**
    * @brief A Byte-sized buffer object
    */
    class byteBuffer {
        private:
            volatile uint8_t *buffer{ nullptr };
            volatile index_t tail{ 0u };
            volatile index_t head{ 0u };
            size_t length{ 0u };
            size_t checkValidPowerOfTwo( size_t k ) noexcept;
            byteBuffer( byteBuffer const& ) = delete;
            void operator=( byteBuffer const& ) = delete;
        public:
            /**
            * @brief Initialize the Byte-sized buffer instance
            * @param[in] pBuffer Block of memory or array of data
            * @param[in] blength The size of the buffer(Must be a power of two)
            */
            byteBuffer( volatile uint8_t *pBuffer, const size_t blength )
            {
                (void)setup( pBuffer, blength );
            }
            /**
            * @brief Initialize the Byte-sized buffer
            * @param[in] pBuffer Block of memory or array of data
            * @param[in] blength The size of the buffer(Must be a power of two)
            * @return @c true on success, otherwise returns @c false.
            */
            bool setup( volatile uint8_t *pBuffer, const size_t blength ) noexcept;
            /**
            * @brief Adds an element of data to the Byte-sized buffer
            * @param[in] bData The data to be added
            * @return @c true on success, otherwise returns @c false.
            */
            bool put( const uint8_t bData ) noexcept;
            /**
            * @brief Gets n data from the Byte-sized buffer and removes them
            * @param[out] dst the location where the data-byte will be written
            * @param[in] n The number of bytes to read
            * @return @c true on success, otherwise returns @c false
            */
            bool read( void *dst, const size_t n ) noexcept;
            /**
            * @brief Gets one data-byte from the front of the Byte-sized buffer, and
            * removes it
            * @param[out] dst the location where the data-byte will be written
            * @return @c true on success, otherwise returns @c false
            */
            bool get( uint8_t *dst ) noexcept;
            /**
            * @brief Looks for one byte from the head of the Byte-sized buffer without
            * removing it
            * @return byte of data, or zero if nothing in the list
            */
            uint8_t peek( void ) const noexcept;
            /**
            * @brief Query the empty status of the Byte-sized buffer
            * @return @c true if the Byte-sized buffer is empty, @c false if it
            * is not.
            */
            bool isEmpty( void ) const noexcept;
            /**
            * @brief Query the full status of the Byte-sized buffer
            * @return @c true if the Byte-sized buffer is empty, @c false if it
            * is not.
            */
            bool isFull( void ) const noexcept;
            /**
            * @brief Query the number of elements in the Byte-sized buffer
            * @return Number of elements in the byte-sized Buffer.
            */
            size_t count( void ) const noexcept;
    };
    
    /** @}*/
}


#endif /*QOS_BYTE_BUFFER*/
