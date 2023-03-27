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
            std::size_t blockSize{ 0u };
        };
        using blockConnect_t = struct _blockConnect_s;

        class pool {
            protected:
                blockConnect_t *end{ nullptr };
                blockConnect_t start;
                std::uint8_t *poolMemory{ nullptr };
                std::size_t poolMemSize{ 0u };
                std::size_t freeBytesRemaining{ 0u };
                void insertBlockIntoFreeList( blockConnect_t *xBlock );
                void init( void );
            public:
                inline pool( void *pArea, const std::size_t pSize ) {
                    (void)setup( pArea, pSize );
                }
                bool setup( void *pArea, const std::size_t pSize );
                void free( void *ptr );
                void* alloc( std::size_t pSize );
                std::size_t getFreeSize( void );
        };

    };
};

    #if ( Q_DEFAULT_HEAP_SIZE >= 64 )
        void * operator new( std::size_t size);
        void * operator new[]( std::size_t size);

        void operator delete(void * ptr) noexcept;
        void operator delete[](void * ptr) noexcept;

        void operator delete(void* ptr, void* place) noexcept;
        void operator delete[](void* ptr, void* place) noexcept;
    #endif /*Q_USE_MEM_ALLOCATION_SCHEME*/

#endif /*QOS_CPP_MEMORY*/