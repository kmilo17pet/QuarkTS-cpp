#include "include/util.hpp"
#include "include/helper.hpp"
#include "config/config.h"

#ifndef  Q_MAX_FTOA_PRECISION
    #define Q_MAX_FTOA_PRECISION    ( 10U )
#endif
#ifndef Q_ATOF_FULL
    #define Q_ATOF_FULL             ( 0 )
#endif

#define Q_IO_UTIL_MAX_STRLEN        ( static_cast<size_t>( 512U ) )

using namespace qOS;
using namespace qOS::util;

static const char * discardWhitespaces( const char *s, size_t maxlen ) noexcept;
static const char * checkStrSign( const char *s, int *sgn ) noexcept;
static size_t xBaseUtoA( unsigned_t num, char* str, uint8_t base ) noexcept;
static char nibbleToX( uint8_t value ) noexcept;
static bool operationIO( const ioFcn_t fcn, void* pStorage, void *pData, const size_t n, bool aip, bool operation ) noexcept;
static uint8_t hexCharToUnsigned( const char c ) noexcept;

/*============================================================================*/
static const char * discardWhitespaces( const char *s, size_t maxlen ) noexcept
{
    /*cstat -MISRAC++2008-5-14-1*/
    /*isspace is known to not have side effects*/
    while ( ( maxlen > 0U ) && ( 0 != isspace( static_cast<int>( *s ) ) ) ) {
        s++;
        --maxlen;
    }
    /*cstat +MISRAC++2008-5-14-1*/
    return s;
}
/*============================================================================*/
static const char * checkStrSign( const char *s, int *sgn ) noexcept
{
    if ( '-' == *s ) {
        *sgn = -1;
        ++s;
    }
    else if ( '+' == *s ) {
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
    } while ( ( '\0' != ( *s++ ) ) && ( maxlen > 0U ) );

    return retValue;
}
/*============================================================================*/
size_t util::strlen( const char* s, size_t maxlen ) noexcept
{
    size_t count;

    if ( ( nullptr == s ) || ( 0U == maxlen ) ) {
        count = 0U;
    }
    else {
        count = 0U;
        while ( ( '\0' != *s ) && ( maxlen > 0U ) ) {
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

    if ( ( srclen + 1U ) < maxlen ) {
        (void)memcpy( dst, src, srclen + 1U );
    }
    else if ( 0U != maxlen ) {
        (void)memcpy( dst, src, maxlen - 1U );
        dst[ maxlen - 1U ] = '\0';
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
            (void)memcpy( &dst[ dstlen ], src, srclen + 1U ) ;
        }
        else {
            (void)memcpy( &dst[ dstlen ], src, maxlen - 1U );
            dst[ dstlen + maxlen - 1U ] = '\0';
        }
        retVal = dstlen + srclen;
    }

    return retVal;
}
/*============================================================================*/
/*perform conversion of unsigned integer to ASCII. NULL Terminator not included*/
static size_t xBaseUtoA( unsigned_t num, char* str, uint8_t base ) noexcept
{
    size_t i = 0U;

    if ( ( 0UL == num ) || ( 0U == base ) ) {
        str[ i++ ] = '0';
    }
    else {
        while ( 0UL != num ) {
            const unsigned long r = num % static_cast<unsigned long>( base );
            /*cstat -CERT-INT30-C_a*/
            str[ i++ ] = ( r > 9UL ) ? /*i++ never wraps*/
                static_cast<char>( static_cast<uint8_t>( r - 10UL ) + static_cast<uint8_t>( 'A' ) ) :
                static_cast<char>( static_cast<uint8_t>( r ) + static_cast<uint8_t>( '0' ) );
            /*cstat +CERT-INT30-C_a*/
            num = num/base;
        }
        (void)util::swapBytes( str, i );
    }

    return i;
}
/*============================================================================*/
static char nibbleToX( uint8_t value ) noexcept
{
    char ch;

    ch = static_cast<char>( ( value & 0x0FU ) + 48U );

    return ( ch <= '9' ) ? ch : static_cast<char>( static_cast<uint8_t>( ch ) + 7U );
}
/*============================================================================*/
bool util::swapBytes( void *pData, const size_t n ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != pData ) && ( n > 0U ) ) {
        /*cstat -CERT-EXP36-C_b -MISRAC++2008-7-1-1*/
        uint8_t *p = static_cast<uint8_t*>( pData );
        /*cstat +CERT-EXP36-C_b +MISRAC++2008-7-1-1*/
        size_t lo;
        size_t hi;

        hi = n - 1U;
        for ( lo = 0U ; hi > lo ; ++lo ) {
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
    const uint16_t i = 1U;
    /*cstat -CERT-INT36-C*/
    return ( reinterpret_cast<const uint8_t*>( &i )[ 0 ] == 1U );
    /*cstat +CERT-INT36-C*/
}
/*============================================================================*/
bool util::outputString( util::putChar_t fcn, const char *s, void* pStorage, bool aip ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != fcn ) && ( nullptr != s ) ) {
        /*cstat -MISRAC++2008-7-1-1 -CERT-EXP36-C_b*/
        char *xPtr = static_cast<char*>( pStorage );
        /*cstat +MISRAC++2008-7-1-1 +CERT-EXP36-C_b*/
        if ( ( aip ) && ( nullptr != xPtr ) ) {
            size_t i = 0U;

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
bool util::printXData( util::putChar_t fcn, void *pData, size_t n, bool eol, void* pStorage ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != fcn ) && ( n > 0U ) ) {
        /*cstat -MISRAC++2008-7-1-1 -CERT-EXP36-C_b*/
        uint8_t *pdat = static_cast<uint8_t*>( pData );
        /*cstat +MISRAC++2008-7-1-1 +CERT-EXP36-C_b*/
        for ( size_t i = 0U ; i < n ; ++i ) {
            fcn( pStorage, nibbleToX( pdat[ i ] >> 4U ) );
            fcn( pStorage, nibbleToX( pdat[ i ] & 0x0FU ) );
            fcn( pStorage, ' ' );
        }
        if ( eol ) {
            fcn( pStorage, '\r' );
            fcn( pStorage, '\n' );
        }
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
static bool operationIO( const ioFcn_t fcn, void* pStorage, void *pData, const size_t n, bool aip, bool operation ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != fcn ) && ( pData != nullptr ) && ( n > 0U ) ) {
        /*cstat -CERT-EXP36-C_b*/
        char *cdata = static_cast<char*>( pData );
        char *iStg = static_cast<char*>( pStorage );
        char *nStg = static_cast<char*>( pStorage );
        /*cstat +CERT-EXP36-C_b*/
        char tmp;
        char *ptmp = &tmp;
        char **dstDat;
        char **dstStg;

        dstDat = ( !operation ) ? &ptmp : &cdata;
        dstStg = ( ( aip ) && ( nullptr != pStorage ) ) ? &iStg : &nStg;
        for ( size_t i = 0U ; i < n ; ++i ) {
            dstDat[ 0 ][ 0 ] = fcn( dstStg[ 0 ] , dstDat[ 0 ][ 0 ] );
            cdata++;
            iStg++;
        }
        retValue = true;
    }
    return retValue;
}
/*============================================================================*/
bool util::outputRAW( const ioFcn_t fcn, void *pData, const size_t n, void* pStorage, bool aip ) noexcept
{
    return operationIO( fcn, pStorage, pData, n, aip, false );
}
/*============================================================================*/
bool util::inputRAW( const ioFcn_t fcn, void *pData, const size_t n, void* pStorage, bool aip ) noexcept
{
    return operationIO( fcn, pStorage, pData, n, aip, true );
}
/*============================================================================*/
static uint8_t hexCharToUnsigned( const char c ) noexcept /* <c> should only contain a valid hex character*/
{
    const uint8_t b = static_cast<uint8_t>( c );
    return static_cast<uint8_t>( ( ( b & 0xFU ) + ( b >> 6U ) ) | ( ( b >> 3U ) & 0x8U ) );
}
/*============================================================================*/
uint32_t util::hexStringToUnsigned( const char *s ) noexcept
{
    uint32_t val = 0UL;

    if ( nullptr != s ) {
        uint8_t nParsed = 0U;

        while ( ( '\0' != *s ) && ( nParsed < 8U ) ) {
            const char c = *s++;
            if ( 0 != isxdigit( static_cast<int>( c ) ) ) {
                val = ( val << 4UL ) | static_cast<uint32_t>( hexCharToUnsigned( c ) );
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
float64_t util::stringToFloat( const char *s ) noexcept
{
    float64_t rez = 0.0;
    float64_t fact;
    bool point_seen = false;
    int sgn = 1;
    char c;

    #if ( Q_ATOF_FULL == 1 )
        int power2 = 0, powerSign = 1;
        float64_t power = 1.0, eFactor;
    #endif

    s = discardWhitespaces( s, Q_IO_UTIL_MAX_STRLEN );
    s = checkStrSign( s, &sgn );
    /*cstat -CERT-FLP36-C*/
    fact = static_cast<float64_t>( sgn );
    /*cstat -MISRAC++2008-6-2-1*/
    while ( '\0' != static_cast<uint8_t>( c = *s ) ) {
        if ( '.' == c ) {
            point_seen = true;
        }
        else if ( 0 != isdigit( static_cast<int>( c ) ) ) {
            if ( point_seen ) {
                fact *= 0.1;
            }
            rez = ( rez * 10.0 ) + ( static_cast<float64_t>( c ) ) - 48.0; /*CERT-FLP36-C deviation allowed*/
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
            eFactor = ( -1 == powerSign ) ? 0.1 : 10.0;
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
        uint32_t u = 0U;

        (void)memcpy( &u, &num, sizeof(uint32_t) );
        u &= 0x7FFFFFFFU;

        if ( 0U == u ) {
            (void)util::strcpy( str, "0.0", 4 );
        }
        else if ( u < 0x7F800000U ) {
            uint32_t intPart;
            size_t i = 0U;

            precision = clipUpper( precision, static_cast<uint8_t>( Q_MAX_FTOA_PRECISION ) );
            if ( num < 0.0F ) {
                num = -num;
                str[ i++ ] = '-';
            }

            intPart = static_cast<uint32_t>( num );
            /*cstat -CERT-FLP36-C*/
            num -= static_cast<float32_t>( intPart );
            i += xBaseUtoA( intPart, &str[ i ], 10U );
            if ( precision > 0U ) {
                str[ i++ ] = '.';
                while ( 0U != precision-- ) {
                    char c;
                    num *= 10.0F;
                    c = static_cast<char>( num );
                    str[ i++ ] = static_cast<char>( static_cast<uint8_t>( c ) + '0' );
                    num -= static_cast<float32_t>( c );
                }
            }
            /*cstat +CERT-FLP36-C*/
            str[ i ] = '\0';
        }
        else if ( 0x7F800000U == u ) {
            str[ 0 ] = ( num > 0.0F ) ? '+' : '-';
            (void)util::strcpy( &str[ 1 ] , "inf", 4 );
        }
        else {
            (void)util::strcpy( str, "nan", 4 );
        }
    }

    return str;
}
/*============================================================================*/
char* util::doubleToString( float64_t num, char* str, uint8_t precision ) noexcept
{
    if ( nullptr != str ) {
        uint64_t u = 0U;

        (void)memcpy( &u, &num, sizeof(uint64_t) );
        u &= 0x7FFFFFFFFFFFFFFFULL;

        if ( 0ULL == u ) {
            (void)util::strcpy( str, "0.0", 4 );
        }
        else if ( u < 0x7FF0000000000000ULL ) {
            uint32_t intPart;
            size_t i = 0U;

            precision = clipUpper( precision, static_cast<uint8_t>( Q_MAX_FTOA_PRECISION ) );
            if ( num < 0.0 ) {
                num = -num;
                str[ i++ ] = '-';
            }

            intPart = static_cast<uint32_t>( num );
            /*cstat -CERT-FLP36-C*/
            num -= static_cast<double>( intPart );
            i += xBaseUtoA( intPart, &str[ i ], 10U );
            if ( precision > 0U ) {
                str[ i++ ] = '.';
                while ( 0U != precision-- ) {
                    char c;
                    num *= 10.0;
                    c = static_cast<char>( num );
                    str[ i++ ] = static_cast<char>( static_cast<uint8_t>( c ) + '0' );
                    num -= static_cast<double>( c );
                }
            }
            /*cstat +CERT-FLP36-C*/
            str[ i ] = '\0';
        }
        else if ( 0x7FF0000000000000ULL == u ) {
            str[ 0 ] = ( num > 0.0 ) ? '+' : '-';
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
        int res = 0;
        int sgn = 1;

        s = discardWhitespaces( s, Q_IO_UTIL_MAX_STRLEN );
        s = checkStrSign( s, &sgn );
        while ( '\0' != *s ) {
            if ( ( *s >= '0' ) && ( *s <= '9' ) ) {
                res = ( res * 10 ) + ( static_cast<int>( *s ) ) - ( static_cast<int>( '0' ) );
                ++s;
            }
            else {
                break;
            }
        }
        retValue = sgn * res;
    }

    return retValue;
}
/*============================================================================*/
char* util::unsignedToString( unsigned_t num, char* str, uint8_t base ) noexcept
{
    if ( nullptr != str ) {
        size_t i;

        i = xBaseUtoA( num, str, base );
        str[ i ] = '\0';
    }

    return str;
}
/*============================================================================*/
char* util::integerToString( signed_t num, char* str, uint8_t base ) noexcept
{
    if ( nullptr != str ) {
        size_t i = 0U;

        if ( num < 0 ) {
            if ( 10U == base ) {
                str[ i++ ] = '-';
            }
            num = -num;
        }
        /*cstat -MISRAC++2008-5-0-9*/
        i += xBaseUtoA( static_cast<unsigned_t>( num ), &str[ i ], base );
        /*cstat +MISRAC++2008-5-0-9*/
        str[ i ] = '\0';
    }

    return str;
}
/*============================================================================*/
char* util::pointerToString( const void *ptr, char* str ) noexcept
{
    if ( nullptr != str ) {
        if ( nullptr != ptr ) {
            size_t i;
            /*cstat -CERT-INT36-C*/
            const auto num = reinterpret_cast<uintptr_t>( ptr );
            /*cstat +CERT-INT36-C*/
            str[ 0 ] = '0';
            str[ 1 ] = 'x';
            i = xBaseUtoA( num, &str[ 2 ], 16 );
            str[ i + 2 ] = '\0';
        }
        else {
            (void)util::strcpy( str, "null", 5 );
        }
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
