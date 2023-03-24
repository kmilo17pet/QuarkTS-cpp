#ifndef QOS_CPP_UTIL
#define QOS_CPP_UTIL

#include "types.hpp"

namespace qOS {

    namespace util {
        using putChar_t = void (*)( void* , const char );
        using ioFcn_t = char (*)( void *, const char );

        char* strchr( const char *s, int c, std::size_t maxlen );
        std::size_t strlen( const char* s, std::size_t maxlen );
        std::size_t strcpy( char * dst, const char * src, std::size_t maxlen );
        std::size_t strcat( char *dst, const char *src, std::size_t maxlen );
        bool swapBytes( void *pData, const std::size_t n );
        bool checkEndianness( void );
        bool outputString( util::putChar_t fcn, void* pStorage, const char *s, bool aip );
        bool printXData( util::putChar_t fcn, void* pStorage, void *pData, std::size_t n );
        bool outputRAW( const ioFcn_t fcn, void* pStorage, void *pData, const std::size_t n, bool aip );
        bool inputRAW( const ioFcn_t fcn, void* pStorage, void *pData, const std::size_t n, bool aip );
        std::uint32_t xtou32( const char *s );
        float64_t atof( const char *s );
        char* ftoa( float32_t num, char *str, std::uint8_t precision );
        int atoi( const char *s );
        char* utoa( std::uint32_t num, char* str, std::uint8_t base );
        char* itoa( std::int32_t num, char* str, std::uint8_t base );
        char* btoa( const bool num, char *str );
    };
}

#endif /*QOS_CPP_UTIL*/