#ifndef QOS_CPP_HELPER
#define QOS_CPP_HELPER

#include "types.hpp"

namespace qOS {

    template <typename T, typename V>
    inline void bitsSet( T &dst, V xBits )
    {
        dst |= xBits;
    }

    template <typename T, typename V>
    inline void bitsClear( T &dst, V xBits )
    {
        dst &= ~xBits;
    }
    /*cstat -CERT-INT34-C_a*/
    template <typename T>
    inline void bitSet( T &dst, const std::size_t xBit )
    {
        dst |= ( static_cast<T>( 1u ) << xBit );
    }

    template <typename T>
    inline void bitClear( T &dst, const std::size_t xBit )
    {
        dst &= ~( static_cast<T>( 1u ) << xBit ); 
    }
    /*cstat +CERT-INT34-C_a*/
    template <typename T>
    inline bool bitRead( T dst, const std::size_t xBit )
    {
        return ( false == ( dst & ( 1u << xBit ) ) ) ? false : true;
    }

    template <typename T>
    inline void bitToggle( T &dst, const std::size_t xBit )
    {
        dst ^= ( 1u << xBit );
    }

    template <typename T>
    inline void bitWrite( T &dst, const std::size_t xBit, const bool value )
    {
        ( value ) ? bitSet( dst, xBit ) : bitClear( dst, xBit );
    }

    template <typename T, typename V>
    inline bool bitsGet( T reg, V xBits )
    {
        return ( 0u != ( reg & xBits) ) ? true : false;
    }

    template <typename T>
    inline T clip( T x, const T Min, const T Max )
    {
        return ( x < Min ) ? Min : ( ( x > Max ) ? Max : x ); 
    }

    template <typename T>
    inline T clipUpper( T x, const T Max )
    {
        return ( x > Max ) ? Max : x ;
    }

    template <typename T>
    inline T clipLower( T x, const T Min )
    {
        return ( x < Min ) ? Min : x ;
    }

    template <typename T>
    inline T isBetween( T x, const T Low, const T High )
    {
        return ( x >= Low ) && ( x <= High );
    }

    inline std::uint8_t byteNibbleHigh( std::uint8_t x )
    {
        return static_cast<std::uint8_t>( x >> 4u );
    }

    inline std::uint8_t byteNibbleLow( std::uint8_t x )
    {
        return static_cast<std::uint8_t>( x & 0x0Fu );
    }

    inline std::uint8_t byteMergeNibbles( std::uint8_t h, std::uint8_t l )
    {
        return static_cast<std::uint8_t>( ( h << 4u ) | ( 0x0Fu & l ) );
    }

    inline std::uint16_t wordByteHigh( std::uint16_t x )
    {
        return static_cast<std::uint16_t>( x >> 8u );
    }

    inline std::uint16_t wordByteLow( std::uint16_t x )
    {
        return static_cast<std::uint16_t>( x & 0x00FFu );
    }

    inline std::uint16_t wordMergeNibbles( std::uint16_t h, std::uint16_t l )
    {
        return static_cast<std::uint16_t>( ( h << 8u ) | ( 0x00FFu & l ) );
    }

    inline std::uint32_t dWordWordHigh( std::uint32_t x )
    {
        return static_cast<std::uint32_t>( x >> 16u );
    }

    inline std::uint32_t dWordWordLow( std::uint32_t x )
    {
        return static_cast<std::uint32_t>( x & 0xFFFFu );
    }

    inline std::uint32_t dwordMergeWords( std::uint32_t h, std::uint32_t l )
    {
        return static_cast<std::uint32_t>( ( h << 16u ) | ( 0xFFFFu & l ) );
    }

}

#endif /*QOS_CPP_HELPER*/
