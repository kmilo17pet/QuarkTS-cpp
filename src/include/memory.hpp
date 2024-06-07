#ifndef QOS_CPP_MEMORY
#define QOS_CPP_MEMORY

#include "include/types.hpp"

#ifndef Q_BYTE_ALIGNMENT
    #define Q_BYTE_ALIGNMENT        ( 8 )
#endif

#if ( ( Q_BYTE_ALIGNMENT != 1 ) && ( Q_BYTE_ALIGNMENT != 2 ) && ( Q_BYTE_ALIGNMENT != 4 ) && ( Q_BYTE_ALIGNMENT != 8 ) )
    #error Q_BYTE_ALIGNMENT value not allowed, use only 1,2,4 or 8(default).
#endif

namespace qOS {

    /**
    * @brief Memory management interfaces.
    */
    namespace mem {

        /** @addtogroup qmemmang
        * @brief API interface for the @ref q_memmang extension.
        *  @{
        */

        /*! @cond */
        using address_t = size_t;
        struct blockConnect_t {
            blockConnect_t *next{ nullptr };
            size_t blockSize{ 0U };
        };
        /*! @endcond */

        /**
        * @brief A Memory Pool object
        * @details A memory pool its a special resource that allows memory blocks to
        * be dynamically allocated from a user-designated memory region. Instead of
        * typical pools with fixed-size block allocation, the pools in QuarkTS++ can
        * be of any size, thereby the user is responsible for selecting the
        * appropriate memory pool to allocate data with the same size.
        */
        class pool {
            private:
                blockConnect_t *end{ nullptr };
                blockConnect_t start;
                uint8_t *poolMemory{ nullptr };
                size_t poolMemSize{ 0U };
                size_t freeBytesRemaining{ 0U };
                void insertBlockIntoFreeList( blockConnect_t *xBlock ) noexcept;
                void init( void ) noexcept;
                pool( pool const& ) = delete;
                void operator=( pool const& ) = delete;
            public:
                /*! @cond  */
                virtual ~pool() {}
                /*! @endcond  */
                /**
                * @brief Initializes a memory pool instance.
                * @param[in] pArea A pointer to a memory block @c uint8_t statically
                * allocated to act as Heap of the memory pool. The size of this block should
                * match the @a pSize argument.
                * @param[in] pSize The size of the memory block pointed by @a pArea
                */
                inline pool( void *pArea, const size_t pSize ) noexcept {
                    (void)setup( pArea, pSize );
                }
                /**
                * @brief Initializes a memory pool instance. This function should be called
                * once before any heap memory request.
                * @param[in] pArea A pointer to a memory block @c uint8_t statically
                * allocated to act as Heap of the memory pool. The size of this block should
                * match the @a pSize argument.
                * @param[in] pSize The size of the memory block pointed by @a pArea
                * @return Returns @c true on success, otherwise, returns @c false.
                */
                bool setup( void *pArea, const size_t pSize ) noexcept;
                /**
                * @brief Deallocates the space previously allocated by mem::pool::alloc().
                * Deallocation will be performed in the selected memory pool.
                * If @a ptr is a @c nullptr pointer, the function does nothing.
                * The behavior is undefined if selected memory pool has not been initialized.
                * The behavior is undefined if the value of @a ptr  does not equal a value
                * returned earlier by mem::pool::alloc().
                * The behavior is undefined if the memory area referred to by @a ptr has
                * already been deallocated, that is, mem::pool::free() has already been called with
                * @a ptr as the argument and no calls to mem::pool::alloc() resulted in a pointer
                * equal to @a ptr afterwards. The behavior is undefined if after mem::pool::free()
                * returns, an access is made through the pointer @a ptr.
                * @attention This method is NOT interrupt-safe.
                * @param[in] ptr to the memory to deallocate
                */
                void free( void *ptr ) noexcept;
                /**
                * @brief Allocate a block of memory that is @a pSize bytes large.
                * If the requested memory can be allocated, a pointer is
                * returned to the beginning of the memory block.
                * @attention This method is NOT interrupt-safe.
                * @param[in] pSize Size of the memory block in bytes.
                * @return If the request is successful then a pointer to the memory block is
                * returned. If the function failed to allocate the requested block of memory
                * , a @c nullptr pointer is returned.
                */
                void* alloc( size_t pSize ) noexcept;
                /**
                * @brief Returns the total amount of heap space that remains unallocated for
                * the memory pool.
                * @return The size of the unallocated heap.
                */
                size_t getFreeSize( void ) const noexcept;
        };

        /** @}*/
    }
}

    #if ( Q_DEFAULT_HEAP_SIZE >= 64 )
        void * operator new( size_t size);
        void * operator new[]( size_t size);

        void operator delete(void * ptr) noexcept;
        void operator delete[](void * ptr) noexcept;

        void operator delete(void* ptr, void* place) noexcept;
        void operator delete[](void* ptr, void* place) noexcept;
    #endif /*Q_USE_MEM_ALLOCATION_SCHEME*/

#endif /*QOS_CPP_MEMORY*/
