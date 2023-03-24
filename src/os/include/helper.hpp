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

    template <typename T>
    inline void bitSet( T &dst, const size_t xBit ) 
    {
        dst |= ( 1u << xBit );
    }

    template <typename T>
    inline void bitClear( T &dst, const size_t xBit ) 
    {
        dst &= ~( 1u << xBit ); 
    }

    template <typename T>
    inline bool bitRead( T dst, const size_t xBit ) 
    {
        return ( false == ( dst & ( 1u << xBit ) ) ) ? false : true;
    }

    template <typename T>
    inline void bitToggle( T &dst, const size_t xBit ) 
    {
        dst ^= ( 1u << xBit );
    }

    template <typename T>
    inline void bitWrite( T &dst, const size_t xBit, const bool value ) 
    {
        ( value ) ? bitSet( dst, xBit ) : bitClear( dst, xBit );
    }

    template <typename T, typename V>
    inline bool bitsGet( T reg, V xBits ) 
    {
        return ( 0u != ( reg & xBits) ) ? true : false;
    }

    template <typename T>
    inline T clipUpper( T x, const T Max ) 
    {
        return ( x > Max ) ? Max : x ;
    }

}

#endif /*QOS_CPP_HELPER*/