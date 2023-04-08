#ifndef QOS_CPP_HELPER
#define QOS_CPP_HELPER

#include "types.hpp"

namespace qOS {

    namespace bits {
        /*cstat -CERT-INT34-C_a -MISRAC++2008-5-0-10*/
        template <typename T, typename V>
        inline void multipleSet( T &dst, V xBits )
        {
            dst |= static_cast<T>( xBits );
        }

        template <typename T, typename V>
        inline void multipleClear( T &dst, V xBits )
        {
            dst &= ~static_cast<T>( xBits );
        }

        template <typename T>
        inline void singleSet( T &dst, const size_t xBit )
        {
            dst |= ( static_cast<T>( 1u ) << static_cast<T>( xBit ) );
        }

        template <typename T>
        inline void singleClear( T &dst, const size_t xBit )
        {
            dst &= ~( static_cast<T>( 1u ) << static_cast<T>( xBit ) );
        }

        template <typename T>
        inline bool singleRead( T dst, const size_t xBit )
        {
            return ( static_cast<T>( 0u ) == ( dst & ( static_cast<T>( 1u ) << static_cast<T>( xBit ) ) ) ) ? false : true;
        }

        template <typename T>
        inline void singleToggle( T &dst, const size_t xBit )
        {
            dst ^= ( static_cast<T>( 1u ) << static_cast<T>( xBit ) );
        }
        /*cstat +CERT-INT34-C_a +MISRAC++2008-5-0-10*/
        template <typename T>
        inline void singleWrite( T &dst, const size_t xBit, const bool value )
        {
            ( value ) ? bitSet( dst, xBit ) : bitClear( dst, xBit );
        }

        template <typename T, typename V>
        inline bool multipleGet( T reg, V xBits )
        {
            return ( 0u != ( reg & xBits) ) ? true : false;
        }

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

    inline uint8_t byteNibbleHigh( uint8_t x )
    {
        return static_cast<uint8_t>( x >> 4u );
    }

    inline uint8_t byteNibbleLow( uint8_t x )
    {
        return static_cast<uint8_t>( x & 0x0Fu );
    }

    inline uint8_t byteMergeNibbles( uint8_t h, uint8_t l )
    {
        return static_cast<uint8_t>( ( h << 4u ) | ( 0x0Fu & l ) );
    }

    inline uint16_t wordByteHigh( uint16_t x )
    {
        return static_cast<uint16_t>( x >> 8u );
    }

    inline uint16_t wordByteLow( uint16_t x )
    {
        return static_cast<uint16_t>( x & 0x00FFu );
    }

    inline uint16_t wordMergeNibbles( uint16_t h, uint16_t l )
    {
        return static_cast<uint16_t>( ( h << 8u ) | ( 0x00FFu & l ) );
    }

    inline uint32_t dWordWordHigh( uint32_t x )
    {
        return static_cast<uint32_t>( x >> 16u );
    }

    inline uint32_t dWordWordLow( uint32_t x )
    {
        return static_cast<uint32_t>( x & 0xFFFFu );
    }

    inline uint32_t dwordMergeWords( uint32_t h, uint32_t l )
    {
        return static_cast<uint32_t>( ( h << 16u ) | ( 0xFFFFu & l ) );
    }

}

#endif /*QOS_CPP_HELPER*/
