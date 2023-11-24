#include "include/bytebuffer.hpp"

using namespace qOS;

/*============================================================================*/
size_t byteBuffer::checkValidPowerOfTwo( size_t k ) noexcept
{
    const size_t r = k;

    if ( 0U != ( ( k - 1U ) & k ) ) {
        k--;
        /*cstat -MISRAC++2008-6-5-4 -CERT-INT34-C_a*/
        for ( index_t i = 1U ; i < ( sizeof(index_t)*8U ) ; i = static_cast<index_t>( i * 2U ) ) {
            k = k | static_cast<size_t>( k >> i );
        }
        /*cstat +MISRAC++2008-6-5-4 +CERT-INT34-C_a*/
        k = static_cast<size_t>( ( k + 1U ) >> 1U );
    }

    return ( k < r ) ? ( k * 2U ) : k;
}
/*============================================================================*/
bool byteBuffer::setup( volatile uint8_t *pBuffer, const size_t bLength ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != pBuffer ) && ( length > 0U ) ) {
        head = 0U;
        tail = 0U;
        buffer = pBuffer;
        length = checkValidPowerOfTwo( bLength );
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool byteBuffer::put( const uint8_t bData ) noexcept
{
    bool retValue = false;

    if ( !isFull() ) {
        buffer[ head % length ] = bData;
        ++head;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool byteBuffer::read( void *dst, const size_t n ) noexcept
{
    bool retValue = false;

    if ( n > 0U ) {
        /*cstat -CERT-EXP36-C_b*/
        uint8_t * const pData = static_cast<uint8_t*>( dst );
        /*cstat +CERT-EXP36-C_b*/
        for ( size_t i = 0U ; i < n ; ++i ) {
            (void)get( &pData[ i ] );
        }
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool byteBuffer::get( uint8_t *dst ) noexcept
{
    bool retValue = false;

    if ( !isEmpty() ) {
        const index_t vTail = static_cast<size_t>( tail );
        *dst = buffer[ vTail % length ];
        ++tail;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
uint8_t byteBuffer::peek( void ) const noexcept
{
    const index_t vTail = static_cast<size_t>( tail );
    return static_cast<uint8_t>( buffer[ vTail % length ] );
}
/*============================================================================*/
bool byteBuffer::isEmpty( void ) const noexcept
{
    return ( 0U == count() );
}
/*============================================================================*/
bool byteBuffer::isFull( void ) const noexcept
{
    return ( length == count() );
}
/*============================================================================*/
size_t byteBuffer::count( void ) const noexcept
{
    const index_t vHead = static_cast<size_t>( head );
    const index_t vTail = static_cast<size_t>( tail );
    return static_cast<size_t>( vHead - vTail );
}
/*============================================================================*/
