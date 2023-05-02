#include "include/response.hpp"
#include "include/util.hpp"

using namespace qOS;

/*============================================================================*/
bool response::setup( char *xLocBuff, const size_t nMax ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != xLocBuff ) && ( nMax > 0u ) ) {
        pattern2Match = xLocBuff;
        maxStrLength = nMax;
        reset();
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
void response::reset( void ) noexcept
{
    patternLength = 0u;
    matchedCount = 0u;
    responseReceived = false;
    timeout.disarm();
}
/*============================================================================*/
responseStatus response::received( const char *pattern, const size_t n, qOS::duration_t t ) noexcept
{
    responseStatus retValue = responseStatus::MISSING;

    if ( ( false == responseReceived ) && ( 0u == patternLength ) ) {
        (void)util::strcpy( pattern2Match, pattern, maxStrLength );
        patternLength = ( 0u == n ) ? util::strlen( pattern, maxStrLength ) : n;
        matchedCount = 0u;
        responseReceived = false;
        if ( t > clock::IMMEDIATE ) {
            (void)timeout.set( t );
        }
    }
    else if ( timeout.expired() ) {
        reset();
        retValue = responseStatus::TIMEOUT;
    }
    else if ( true == responseReceived ) {
        reset();
        retValue = responseStatus::SUCCESS;
    }
    else {
        /*nothing to do*/
    }

    return retValue;
}
/*============================================================================*/
bool response::isrHandler( const char rxChar ) noexcept
{
    bool retValue = false;

    if ( ( false == responseReceived ) && ( patternLength > 0u ) ) {
        if ( pattern2Match[ matchedCount ] == rxChar ) {
            ++matchedCount;
            if ( matchedCount == patternLength ) {
                responseReceived = true;
                retValue = responseReceived;
            }
        }
    }

    return retValue;
}
/*============================================================================*/
bool response::isInitialized( void ) const noexcept
{
    return ( maxStrLength > 0u );
}
/*============================================================================*/
