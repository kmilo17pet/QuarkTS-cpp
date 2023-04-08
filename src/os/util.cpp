#include "util.hpp"
#include "helper.hpp"
#include "config.h"

#ifndef  Q_MAX_FTOA_PRECISION
    #define Q_MAX_FTOA_PRECISION    ( 10u )
#endif
#ifndef Q_ATOF_FULL
    #define Q_ATOF_FULL             ( 0 )
#endif

#define Q_IO_UTIL_MAX_STRLEN        ( static_cast<size_t>( 512u ) )

using namespace qOS;
using namespace qOS::util;

static const char * discardWhitespaces( const char *s, size_t maxlen ) noexcept;
static const char * checkStrSign( const char *s, int *sgn ) noexcept;
static size_t xBaseU32toA( uint32_t num, char* str, uint8_t base ) noexcept;
static char nibbleToX( uint8_t value ) noexcept;
static bool operationIO( const ioFcn_t fcn, void* pStorage, void *pData, const size_t n, bool aip, bool operation ) noexcept;
static uint8_t hexCharToUnsigned( const char c ) noexcept;

/*============================================================================*/
static const char * discardWhitespaces( const char *s, size_t maxlen ) noexcept
{
    /*cstat -MISRAC++2008-5-14-1*/
    /*isspace is known to not have side effects*/
    while ( ( maxlen > 0u ) && ( 0 != isspace( static_cast<int>( *s ) ) ) ) {
        s++; /*discard whitespaces*/
        --maxlen;
    }
    /*cstat +MISRAC++2008-5-14-1*/
    return s;
}
/*============================================================================*/
static const char * checkStrSign( const char *s, int *sgn ) noexcept
{
    if ( '-' == *s ) { /*if negative found*/
        *sgn = -1; /*set the sign*/
        ++s; /*move to next*/
    }
    else if ( '+' == *s ) {
        /*plus sign ignored, move to next*/
        ++s;
    }
    else {
        /*nothing to do*/
    }

    return s;
}
/*============================================================================*/
char* util::strchr( const char *s, int c, size_t maxlen ) noexcept
{
    char *retValue = nullptr;
    do {
        if ( static_cast<int>( *s ) == c ) {
            retValue = const_cast<char*>( s );
            break;
        }
        --maxlen;
    } while ( ( '\0' != ( *s++ ) ) && ( maxlen > 0u ) );

    return retValue;
}
/*============================================================================*/
size_t util::strlen( const char* s, size_t maxlen ) noexcept
{
    size_t count;

    if ( ( nullptr == s ) || ( 0u == maxlen ) ) {
        count = 0u;
    }
    else {
        count = 0u;
        while ( ( '\0' != *s ) && ( maxlen > 0u ) ) {
            ++count;
            --maxlen;
            ++s;
        }
    }

    return count;
}
/*============================================================================*/
size_t util::strcpy( char * dst, const char * src, size_t maxlen ) noexcept
{
    const size_t srclen = util::strlen( src, Q_IO_UTIL_MAX_STRLEN );

    if ( ( srclen + 1u ) < maxlen ) {
        (void)memcpy( dst, src, srclen + 1u );
    }
    else if ( 0u != maxlen ) {
        (void)memcpy( dst, src, maxlen - 1u );
        dst[ maxlen - 1u ] = '\0';
    }
    else {
        /*nothing to do here*/
    }

    return srclen;
}
/*============================================================================*/
size_t util::strcat( char *dst, const char *src, size_t maxlen ) noexcept
{
    const size_t srclen = util::strlen( src, Q_IO_UTIL_MAX_STRLEN );
    const size_t dstlen = util::strlen( dst, maxlen);
    size_t retVal;

    if ( dstlen == maxlen ) {
        retVal = maxlen + srclen;
    }
    else {
        if ( srclen < ( maxlen - dstlen ) ) {
            (void)memcpy( &dst[ dstlen ], src, srclen + 1u ) ;
        } 
        else {
            (void)memcpy( &dst[ dstlen ], src, maxlen - 1u );
            dst[ dstlen + maxlen - 1u ] = '\0';
        }
        retVal = dstlen + srclen;
    }

    return retVal;
}
/*============================================================================*/
/*perform conversion of unsigned integer to ASCII. NULL Terminator not included*/
static size_t xBaseU32toA( uint32_t num, char* str, uint8_t base ) noexcept
{
    size_t i = 0u;

    /* Handle 0 explicitly, otherwise empty string is printed for 0 */
    if ( ( 0uL == num ) || ( 0u == base ) ) {
        str[ i++ ] = '0';
    }
    else {
        while ( 0uL != num ) { /*Process individual digits*/
            const uint32_t r = num % static_cast<uint32_t>( base );
            /*cstat -CERT-INT30-C_a*/
            str[ i++ ] = ( r > 9uL ) ? /*i++ never wraps*/
                static_cast<char>( static_cast<uint8_t>( r - 10uL ) + static_cast<uint8_t>( 'A' ) ) :
                static_cast<char>( static_cast<uint8_t>( r ) + static_cast<uint8_t>( '0' ) );
            /*cstat +CERT-INT30-C_a*/
            num = num/base;
        }
        (void)util::swapBytes( str, i );/*Reverse the string*/
    }

    return i;
}
/*============================================================================*/
static char nibbleToX( uint8_t value ) noexcept
{
    char ch;

    ch = static_cast<char>( ( value & 0x0Fu ) + 48u );

    return ( ch <= '9' ) ? ch : static_cast<char>( static_cast<uint8_t>( ch ) + 7u );
}
/*============================================================================*/
bool util::swapBytes( void *pData, const size_t n ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != pData ) && ( n > 0u ) ) {
        /*cstat -CERT-EXP36-C_b -MISRAC++2008-7-1-1*/
        uint8_t *p = static_cast<uint8_t*>( pData );
        /*cstat +CERT-EXP36-C_b +MISRAC++2008-7-1-1*/
        size_t lo;
        size_t hi;

        hi = n - 1u;
        for ( lo = 0u ; hi > lo ; ++lo ) {
            const uint8_t tmp = p[ lo ];
            p[ lo ] = p[ hi ];
            p[ hi ] = tmp;
            --hi;
        }
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool util::checkEndianness( void ) noexcept
{
    const uint16_t i = 1u;
    /*cstat -CERT-INT36-C*/
    return static_cast<bool>( *reinterpret_cast<uint8_t*>( i ) );
    /*cstat +CERT-INT36-C*/
}
/*============================================================================*/
bool util::outputString( util::putChar_t fcn, void* pStorage, const char *s, bool aip ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != fcn ) && ( nullptr != s ) ) {
        /*cstat -MISRAC++2008-7-1-1 -CERT-EXP36-C_b*/
        char *xPtr = static_cast<char*>( pStorage );
        /*cstat +MISRAC++2008-7-1-1 +CERT-EXP36-C_b*/
        if ( ( true == aip ) && ( nullptr != xPtr ) ) {
            size_t i = 0u;

            while ( '\0' != *s ) {
                fcn( &xPtr[ i ] , *s++ );
                ++i;
            }
            retValue = true;
        }
        else {
            while ( '\0' != *s ) {
                fcn( pStorage, *s++ );
            }
            retValue = true;
        }
    }

    return retValue;
}
/*============================================================================*/
bool util::printXData( util::putChar_t fcn, void* pStorage, void *pData, size_t n ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != fcn ) && ( n > 0u ) ) {
        /*cstat -MISRAC++2008-7-1-1 -CERT-EXP36-C_b*/
        uint8_t *pdat = static_cast<uint8_t*>( pData );
        /*cstat +MISRAC++2008-7-1-1 +CERT-EXP36-C_b*/
        for ( size_t i = 0u ; i < n ; ++i ) {
            fcn( pStorage, nibbleToX( pdat[ i ] >> 4u ) );
            fcn( pStorage, nibbleToX( pdat[ i ] & 0x0Fu ) );
            fcn( pStorage, ' ' );
        }
        fcn( pStorage, '\r' );
        fcn( pStorage, '\n' );
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
static bool operationIO( const ioFcn_t fcn, void* pStorage, void *pData, const size_t n, bool aip, bool operation ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != fcn ) && ( pData != nullptr ) && ( n > 0u ) ) {
        /*cstat -CERT-EXP36-C_b*/
        char *cdata = static_cast<char*>( pData );
        char *iStg = static_cast<char*>( pStorage );
        char *nStg = static_cast<char*>( pStorage );
        /*cstat +CERT-EXP36-C_b*/
        char tmp;
        char *ptmp = &tmp;
        char **dstDat, **dstStg;

        /* Operation -> Output=qFalse : Input=qTrue */
        dstDat = ( false == operation ) ? &ptmp : &cdata;
        dstStg = ( ( true == aip ) && ( nullptr != pStorage ) ) ? &iStg : &nStg;

        for ( size_t i = 0u ; i < n ; ++i ) {
            dstDat[ 0 ][ 0 ] = fcn( dstStg[ 0 ] , dstDat[ 0 ][ 0 ] );
            cdata++;
            iStg++;
        }
        retValue = true;
    }
    return retValue;
}
/*============================================================================*/
bool outputRAW( const ioFcn_t fcn, void* pStorage, void *pData, const size_t n, bool aip ) noexcept
{
    return operationIO( fcn, pStorage, pData, n, aip, false );
}
/*============================================================================*/
bool inputRAW( const ioFcn_t fcn, void* pStorage, void *pData, const size_t n, bool aip ) noexcept
{
    return operationIO( fcn, pStorage, pData, n, aip, true );
}
/*============================================================================*/
static uint8_t hexCharToUnsigned( const char c ) noexcept /* <c> should only contain a valid hex character*/
{
    const uint8_t b = static_cast<uint8_t>( c );
    return static_cast<uint8_t>( ( ( b & 0xFu ) + ( b >> 6u ) ) | ( ( b >> 3u ) & 0x8u ) );
}
/*============================================================================*/
uint32_t util::hexStringToUnsigned( const char *s ) noexcept
{
    uint32_t val = 0uL;

    if ( nullptr != s ) {
        uint8_t nParsed = 0u;
        /*
        loop until the end of the string or the number of parsed chars exceeds
        the 32bit notation
        */
        while ( ( '\0' != *s ) && ( nParsed < 8u ) ) {
            const char c = *s++;
            if ( 0 != isxdigit( static_cast<int>( c ) ) ) {
                val = ( val << 4uL ) | static_cast<uint32_t>( hexCharToUnsigned( c ) );
                ++nParsed;
            }
            else if ( 0 != isspace( static_cast<int>( c ) ) ) {
                /*discard any white-space character*/
            }
            else {
                break; /*not valid character, break the conversion*/
            }
        }
    }

    return val;
}
/*============================================================================*/
float32_t util::stringToFloat( const char *s ) noexcept
{
    float32_t rez = 0.0f, fact;
    bool point_seen = false;
    int sgn = 1;
    char c;

    #if ( Q_ATOF_FULL == 1 )
        int power2 = 0, powerSign = 1;
        float32_t power = 1.0f, eFactor;
    #endif

    s = discardWhitespaces( s, Q_IO_UTIL_MAX_STRLEN );
    s = checkStrSign( s, &sgn );
    /*cstat -CERT-FLP36-C*/
    fact = static_cast<float32_t>( sgn );
    /*cstat -MISRAC++2008-6-2-1*/
    while ( '\0' != static_cast<uint8_t>( c = *s ) ) {
        if ( '.' == c ) {
            point_seen = true;
        }
        else if ( 0 != isdigit( static_cast<int>( c ) ) ) {
            if ( true == point_seen ) {
                fact *= 0.1f;
            }
            rez = ( rez * 10.0f ) + ( static_cast<float32_t>( c ) ) - 48.0f; /*CERT-FLP36-C deviation allowed*/
        }
        else {
            break;
        }
        s++;
    }
    /*cstat +MISRAC++2008-6-2-1 +CERT-FLP36-C */
    #if ( Q_ATOF_FULL == 1 )
    if ( ( 'e' == *s ) || ( 'E' == *s ) ) {
        s++;
        if ( ( '-' == *s ) || ( '+' == *s ) ) {
            /*cstat -MISRAC++2008-5-0-3*/
            powerSign = ( '-' == *s ) ? -1 : 1;
            /*cstat +MISRAC++2008-5-0-3*/
            s++;
        }
        while ( 0 != isdigit( static_cast<int>( *s ) ) ) {
            power2 = ( 10*power2 ) + ( *s - '0' );
            s++;
        }
        if ( power2 > 0 ) {
            eFactor = ( -1 == powerSign ) ? 0.1f : 10.0f;
            while ( 0 != power2 ) {
                power *= eFactor;
                --power2;
            }
        }
    }

    return power * rez * fact;
    #else
        return rez * fact;
    #endif
}
/*============================================================================*/
char* util::floatToString( float32_t num, char *str, uint8_t precision ) noexcept
{
    if ( nullptr != str ) {
        uint32_t u = 0u;

        (void)memcpy( &u, &num, sizeof(uint32_t) );
        u &= 0x7FFFFFFFu;

        if ( 0u == u ) {
            (void)util::strcpy( str, "0.0", 4 );
        }
        else if ( u < 0x7F800000u ) {
            uint32_t intPart;
            size_t i = 0u;

            precision = clipUpper( precision, static_cast<uint8_t>( Q_MAX_FTOA_PRECISION ) );
            if ( num < 0.0f ) {
                num = -num;
                str[ i++ ] = '-';
            }

            intPart = static_cast<uint32_t>( num );
            /*cstat -CERT-FLP36-C*/
            num -= static_cast<float32_t>( intPart );
            i += xBaseU32toA( intPart, &str[ i ], 10u );
            if ( precision > 0u ) {
                str[ i++ ] = '.';
                while ( 0u != precision-- ) {
                    char c;
                    num *= 10.0f;
                    c = static_cast<char>( num );
                    str[ i++ ] = static_cast<char>( static_cast<uint8_t>( c ) + '0' );
                    num -= static_cast<float32_t>( c );
                }
            }
            /*cstat +CERT-FLP36-C*/
            str[ i ] = '\0';
        }
        else if ( 0x7F800000u == u ) {
            str[ 0 ] = ( num > 0.0f ) ? '+' : '-';
            (void)util::strcpy( &str[ 1 ] , "inf", 4 );
        }
        else {
            (void)util::strcpy( str, "nan", 4 );
        }
    }

    return str;
}
/*============================================================================*/
int util::stringToInt( const char *s ) noexcept
{
    int retValue = 0;

    if ( nullptr != s ) {
        int res = 0; /*holds the resulting integer*/
        int sgn = 1; /*only to hold the sign*/

        s = discardWhitespaces( s, Q_IO_UTIL_MAX_STRLEN );
        s = checkStrSign( s, &sgn );
        while ( '\0' != *s ) { /*iterate until null char is found*/
            if ( ( *s >= '0' ) || ( *s <= '9' ) ) {
                /*if the char is digit, compute the resulting integer*/
                res = ( res * 10 ) + ( static_cast<int>( *s ) ) - ( static_cast<int>( '0' ) );
                ++s;
            }
            else {
                break;
            }
        }
        retValue = sgn * res; /*return the computed integer with sign*/
    }

    return retValue;
}
/*============================================================================*/
char* util::unsignedToString( uint32_t num, char* str, uint8_t base ) noexcept
{
    if ( nullptr != str ) {
        size_t i;

        /*make the unsigned conversion without the null terminator*/
        i = xBaseU32toA( num, str, base );
        str[ i ] = '\0';
    }

    return str;
}
/*============================================================================*/
char* util::integerToString( int32_t num, char* str, uint8_t base ) noexcept
{
    if ( nullptr != str ) {
        size_t i = 0u;

        if ( num < 0 ) {
            if ( 10u == base ) { /*handle negative numbers only with 10-base*/
                /*put the sign at the beginning*/
                str[ i++ ] = '-';
            }
            num = -num;
        }
        /*make the unsigned conversion without the null terminator*/
        /*cstat -MISRAC++2008-5-0-9*/
        i += xBaseU32toA( static_cast<uint32_t>( num ), &str[ i ], base );
        /*cstat +MISRAC++2008-5-0-9*/
        /*Append string terminator*/
        str[ i ] = '\0';
    }

    return str;
}
/*============================================================================*/
char* util::boolToString( const bool num, char *str ) noexcept
{
    if ( nullptr != str ) {
        ( num ) ? (void)util::strcpy( str, "true", 5 )
                : (void)util::strcpy( str, "false", 6 );
    }

    return str;
}
/*============================================================================*/
