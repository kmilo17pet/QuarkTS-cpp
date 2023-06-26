#include "include/memory.hpp"
#include "include/helper.hpp"

using namespace qOS;


static const size_t BYTE_ALIGN_MASK = static_cast<size_t>( Q_BYTE_ALIGNMENT ) - static_cast<size_t>( 1u );
static const size_t ALLOC_BIT_SEL = ( sizeof(size_t)*static_cast<size_t>( 8u ) ) - static_cast<size_t>( 1u );
static const size_t BLOCK_ALLOCATED_BIT = static_cast<size_t>( 1u ) << ALLOC_BIT_SEL;
static const size_t HEAP_STRUCT_SIZE = ( sizeof(mem::blockConnect_t) + ( BYTE_ALIGN_MASK - static_cast<size_t>( 1u ) ) ) & ~BYTE_ALIGN_MASK;

/*============================================================================*/
bool mem::pool::setup( void *pArea, const size_t pSize ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != pArea ) && ( pSize > 0u ) ) {
        /*cstat -CERT-EXP36-C_b*/
        poolMemory = static_cast<uint8_t*>( pArea );
        /*cstat +CERT-EXP36-C_b*/
        poolMemSize = pSize;
        freeBytesRemaining = pSize;
        end = nullptr;
        init();
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
void mem::pool::insertBlockIntoFreeList( blockConnect_t *xBlock ) noexcept
{
    mem::blockConnect_t *iterator;
    uint8_t *ptr;

    for ( iterator = &start ; iterator->next < xBlock ; iterator = iterator->next ) {}
    ptr = reinterpret_cast<uint8_t*>( iterator );
    /*cstat -SEC-NULL-cmp-bef -PTR-null-cmp-bef -CERT-EXP34-C_g*/
    if ( &ptr[ iterator->blockSize ] == reinterpret_cast<uint8_t*>( xBlock ) ) {
        iterator->blockSize += xBlock->blockSize;
        xBlock = iterator;
    }
    ptr = reinterpret_cast<uint8_t*>( xBlock );
    if ( &ptr[ xBlock->blockSize ] == reinterpret_cast<uint8_t*>( iterator->next ) ) {
        if ( iterator->next != end ) {
            xBlock->blockSize += iterator->next->blockSize;
            xBlock->next = iterator->next->next;
        }
        else {
            xBlock->next = end;
        }
    }
    else {
        xBlock->next = iterator->next;
    }
    /*cstat +SEC-NULL-cmp-bef +PTR-null-cmp-bef +CERT-EXP34-C_g*/
    if ( iterator != xBlock ) {
        iterator->next = xBlock;
    }
}
/*============================================================================*/
void mem::pool::free( void *ptr ) noexcept
{
    /*cstat -CERT-EXP36-C_b*/
    uint8_t *pToFree = static_cast<uint8_t*>( ptr );
    /*cstat +CERT-EXP36-C_b*/
    if ( nullptr != ptr ) {
        mem::blockConnect_t *xConnect;

        pToFree -= HEAP_STRUCT_SIZE;
        xConnect = aligned_cast<mem::blockConnect_t*>( pToFree );
        if ( 0u != ( xConnect->blockSize & BLOCK_ALLOCATED_BIT ) ) {
            if ( NULL == xConnect->next ) {
                bits::multipleClear( xConnect->blockSize, BLOCK_ALLOCATED_BIT );
                freeBytesRemaining += xConnect->blockSize;
                insertBlockIntoFreeList( xConnect );
            }
        }
    }
}
/*============================================================================*/
void mem::pool::init( void ) noexcept
{
    mem::blockConnect_t *firstFreeBlock;
    uint8_t *aligned;
    mem::address_t address, xAddrTmp;
    size_t totalPoolSize = poolMemSize;

    /*cstat -CERT-INT36-C -CERT-EXP39-C_d -CERT-EXP36-C_a*/
    address = reinterpret_cast<mem::address_t>( poolMemory );

    if ( 0uL != ( address & BYTE_ALIGN_MASK ) ) {
        address += BYTE_ALIGN_MASK;
        address &= ~BYTE_ALIGN_MASK;
        totalPoolSize -= address - reinterpret_cast<mem::address_t>( poolMemory );
    }

    aligned = reinterpret_cast<uint8_t*>( address );
    start.next = aligned_cast<mem::blockConnect_t*>( aligned );
    start.blockSize = static_cast<size_t>( 0u );
    xAddrTmp = reinterpret_cast<address_t>( aligned );
    address = xAddrTmp + totalPoolSize;
    address -= HEAP_STRUCT_SIZE;
    address &= ~BYTE_ALIGN_MASK;
    end = reinterpret_cast<mem::blockConnect_t*>( address );
    end->next = nullptr;
    end->blockSize = static_cast<size_t>( 0u );
    firstFreeBlock = aligned_cast<mem::blockConnect_t*>( aligned );
    xAddrTmp = reinterpret_cast<mem::address_t>( firstFreeBlock );
    /*cstat +CERT-INT36-C +CERT-EXP39-C_d +CERT-EXP36-C_a*/
    firstFreeBlock->blockSize = address - xAddrTmp;
    firstFreeBlock->next = end;
    freeBytesRemaining = firstFreeBlock->blockSize;
}
/*============================================================================*/
void* mem::pool::alloc( size_t pSize ) noexcept
{
    void *pAllocated = nullptr;

    if ( pSize > static_cast<size_t>( 0u ) ) {
        const size_t additional = HEAP_STRUCT_SIZE + Q_BYTE_ALIGNMENT - ( pSize & BYTE_ALIGN_MASK );
        if ( pSize > ( ( ~static_cast<size_t>( 0u ) ) - additional ) ) {
            pSize = static_cast<size_t>( 0u );
        }
        else {
            /*cstat -ATH-overflow*/
            pSize += additional;
            /*cstat +ATH-overflow*/
        }
    }

    if ( static_cast<size_t>( 0u ) == ( pSize & BLOCK_ALLOCATED_BIT ) ) {
        if ( ( pSize > static_cast<size_t>( 0u ) ) && ( pSize < freeBytesRemaining ) ) {
            mem::blockConnect_t *xBlock, *previousBlock;
    
            previousBlock = &start;
            xBlock = start.next;
            while ( ( xBlock->blockSize < pSize ) && ( nullptr != xBlock->next ) ) {
                previousBlock = xBlock;
                xBlock = xBlock->next;
            }
            if ( xBlock != end ) {
                const size_t minBlockSize = ( HEAP_STRUCT_SIZE << static_cast<size_t>( 1u ) );

                pAllocated = static_cast<void*>( ( reinterpret_cast<uint8_t*>( previousBlock->next ) ) + HEAP_STRUCT_SIZE );
                previousBlock->next = xBlock->next;
                if ( ( xBlock->blockSize - pSize ) > minBlockSize ) {
                    mem::blockConnect_t *newBlockLink;
                    /*cstat -MISRAC++2008-7-1-1*/
                    uint8_t *pBlockU8 = reinterpret_cast<uint8_t*>( xBlock );
                    uint8_t *ptrBlock = &pBlockU8[ pSize ];
                    /*cstat -CERT-EXP39-C_d -CERT-EXP36-C_a +MISRAC++2008-7-1-1*/
                    newBlockLink = aligned_cast<mem::blockConnect_t*>( ptrBlock );
                    /*cstat +CERT-EXP39-C_d +CERT-EXP36-C_a*/
                    newBlockLink->blockSize = xBlock->blockSize - pSize;
                    /*cstat -ATH-overflow*/
                    xBlock->blockSize = pSize;
                    /*cstat +ATH-overflow*/
                    insertBlockIntoFreeList( newBlockLink );
                }
                freeBytesRemaining -= xBlock->blockSize;
                bits::multipleSet( xBlock->blockSize, BLOCK_ALLOCATED_BIT );
                xBlock->next = nullptr;
            }
        }
    }

    return pAllocated;
}
/*============================================================================*/
size_t mem::pool::getFreeSize( void ) const noexcept
{
    size_t retValue = poolMemSize;

    if ( nullptr != end ) {
        retValue = freeBytesRemaining;
    }

    return retValue;
}
/*============================================================================*/

#if ( Q_DEFAULT_HEAP_SIZE >= 64 )

static uint8_t defaultPoolMemory[ Q_DEFAULT_HEAP_SIZE ] = { 0 };
static mem::pool defaultMemPool( defaultPoolMemory, Q_DEFAULT_HEAP_SIZE );

/*============================================================================*/
void * operator new(size_t size)
{
    return defaultMemPool.alloc( size );
}
/*============================================================================*/
void * operator new[]( size_t size )
{
    return operator new(size);
}
/*============================================================================*/
/*cstat -MISRAC++2008-7-1-2 -MISRAC++2008-0-1-11 -CPU-delete-void*/
void * operator new( size_t size, void *place ) noexcept
{
    /* Nothing to do */
    (void)size;
    return place;
}
/*============================================================================*/
void * operator new[]( size_t size, void *place ) noexcept
{
    return operator new(size, place);
}
/*============================================================================*/
void operator delete( void * ptr ) noexcept
{
    defaultMemPool.free( ptr );
}
/*============================================================================*/
void operator delete[](void * ptr) noexcept
{
    operator delete( ptr );
}
/*============================================================================*/
void operator delete(void* ptr, void* place) noexcept
{
    (void)ptr; 
    (void)place;
    /*Nothing to do*/
}
/*============================================================================*/
void operator delete[](void* ptr, void* place) noexcept
{
    (void)ptr;
    (void)place; // unused
    /*Nothing to do*/
}
/*cstat +MISRAC++2008-7-1-2 +MISRAC++2008-0-1-11 +CPU-delete-void*/
/*============================================================================*/
#endif /*Q_USE_MEM_ALLOCATION_SCHEME*/
