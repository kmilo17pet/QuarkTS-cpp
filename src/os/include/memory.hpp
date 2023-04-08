#ifndef QOS_CPP_MEMORY
#define QOS_CPP_MEMORY

#include "types.hpp"
#include "timer.hpp"

#ifndef Q_BYTE_ALIGNMENT
    #define Q_BYTE_ALIGNMENT        ( 8 )
#endif

#if ( ( Q_BYTE_ALIGNMENT != 1 ) && ( Q_BYTE_ALIGNMENT != 2 ) && ( Q_BYTE_ALIGNMENT != 4 ) && ( Q_BYTE_ALIGNMENT != 8 ) )
    #error Q_BYTE_ALIGNMENT value not allowed, use only 1,2,4 or 8(default).
#endif

namespace qOS {
    namespace mem {

        using address_t = size_t;
        struct _blockConnect_s {
            struct _blockConnect_s *next{ nullptr };
            size_t blockSize{ 0u };
        };
        using blockConnect_t = struct _blockConnect_s;

        class pool {
            private:
                blockConnect_t *end{ nullptr };
                blockConnect_t start;
                uint8_t *poolMemory{ nullptr };
                size_t poolMemSize{ 0u };
                size_t freeBytesRemaining{ 0u };
                void insertBlockIntoFreeList( blockConnect_t *xBlock ) noexcept;
                void init( void ) noexcept;
                pool( pool const& ) = delete;
                void operator=( pool const& ) = delete;
            public:
                inline pool( void *pArea, const size_t pSize ) noexcept {
                    (void)setup( pArea, pSize );
                }
                bool setup( void *pArea, const size_t pSize ) noexcept;
                void free( void *ptr ) noexcept;
                void* alloc( size_t pSize ) noexcept;
                size_t getFreeSize( void ) noexcept;
        };

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
