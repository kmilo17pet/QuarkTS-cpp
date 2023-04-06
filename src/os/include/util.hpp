#ifndef QOS_CPP_UTIL
#define QOS_CPP_UTIL

#include "types.hpp"

namespace qOS {

    namespace util {
        using putChar_t = void (*)( void* , const char );
        using ioFcn_t = char (*)( void *, const char );

        char* strchr( const char *s, int c, std::size_t maxlen ) noexcept;
        std::size_t strlen( const char* s, std::size_t maxlen ) noexcept;
        std::size_t strcpy( char * dst, const char * src, std::size_t maxlen ) noexcept;
        std::size_t strcat( char *dst, const char *src, std::size_t maxlen ) noexcept;
        bool swapBytes( void *pData, const std::size_t n ) noexcept;
        bool checkEndianness( void ) noexcept;
        bool outputString( util::putChar_t fcn, void* pStorage, const char *s, bool aip = false ) noexcept;
        bool printXData( util::putChar_t fcn, void* pStorage, void *pData, std::size_t n ) noexcept;
        bool outputRAW( const ioFcn_t fcn, void* pStorage, void *pData, const std::size_t n, bool aip = false ) noexcept;
        bool inputRAW( const ioFcn_t fcn, void* pStorage, void *pData, const std::size_t n, bool aip = false ) noexcept;
        std::uint32_t hexStringToUnsigned( const char *s ) noexcept;
        float32_t stringToFloat( const char *s ) noexcept;
        char* floatToString( float32_t num, char *str, std::uint8_t precision = 10u ) noexcept;
        int stringToInt( const char *s ) noexcept;
        char* unsignedToString( std::uint32_t num, char* str, std::uint8_t base = 10u ) noexcept;
        char* integerToString( std::int32_t num, char* str, std::uint8_t base = 10u ) noexcept;
        char* boolToString( const bool num, char *str ) noexcept;
    }
}

#endif /*QOS_CPP_UTIL*/
