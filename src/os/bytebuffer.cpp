#include "byteBuffer.hpp"

using namespace qOS;

/*============================================================================*/
std::size_t byteBuffer::checkValidPowerOfTwo( std::size_t k ) noexcept
{
    const std::size_t r = k;

    if ( 0u != ( ( k - 1u ) & k ) ) {
        k--;
        /*cstat -MISRAC++2008-6-5-4 -CERT-INT34-C_a*/
        for ( index_t i = 1u ; i < ( sizeof(index_t)*8u ) ; i = static_cast<index_t>( i * 2u ) ) {
            k = k | static_cast<std::size_t>( k >> i );
        }
        /*cstat +MISRAC++2008-6-5-4 +CERT-INT34-C_a*/
        k = static_cast<std::size_t>( ( k + 1u ) >> 1u );
    }

    return ( k < r ) ? ( k * 2u ) : k;
}
/*============================================================================*/
bool byteBuffer::setup( volatile std::uint8_t *pBuffer, const size_t bLength ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != pBuffer ) && ( length > 0u ) ) {
        head = 0u;
        tail = 0u;
        buffer = pBuffer;
        length = checkValidPowerOfTwo( bLength );
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool byteBuffer::put( const std::uint8_t bData ) noexcept
{
    bool retValue = false;

    if ( false == isFull() ) {
        buffer[ head % length ] = bData;
        ++head;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool byteBuffer::read( void *dst, const std::size_t n ) noexcept
{
    bool retValue = false;

    if ( n > 0u ) {
        /*cstat -CERT-EXP36-C_b*/
        std::uint8_t * const pData = static_cast<std::uint8_t*>( dst );
        /*cstat +CERT-EXP36-C_b*/
        for ( std::size_t i = 0u ; i < n ; ++i ) {
            (void)get( &pData[ i ] );
        }
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool byteBuffer::get( std::uint8_t *dst ) noexcept
{
    bool retValue = false;

    if ( false == isEmpty() ) {
        const index_t vTail = static_cast<std::size_t>( tail );
        *dst = buffer[ vTail % length ];
        ++tail;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
std::uint8_t byteBuffer::peek( void ) const noexcept
{
    const index_t vTail = static_cast<std::size_t>( tail );
    return static_cast<std::uint8_t>( buffer[ vTail % length ] );
}
/*============================================================================*/
bool byteBuffer::isEmpty( void ) const noexcept
{
    return ( 0u == count() );
}
/*============================================================================*/
bool byteBuffer::isFull( void ) const noexcept
{
    return ( length == count() );
}
/*============================================================================*/
std::size_t byteBuffer::count( void ) const noexcept
{
    const index_t vHead = static_cast<std::size_t>( head );
    const index_t vTail = static_cast<std::size_t>( tail );
    return static_cast<std::size_t>( vHead - vTail );
}
/*============================================================================*/
