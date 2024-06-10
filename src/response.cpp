#include "include/response.hpp"
#include "include/util.hpp"
#include "include/helper.hpp"

using namespace qOS;

/*============================================================================*/
bool response::setup( char *xLocBuff, const size_t nMax ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != xLocBuff ) && ( nMax > 0U ) ) {
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
    patternLength = 0U;
    matchedCount = 0U;
    responseReceived = false;
    timeout.disarm();
}
/*============================================================================*/
responseStatus response::received( const char *pattern, const size_t n, const qOS::duration_t t ) noexcept
{
    responseStatus retValue = responseStatus::MISSING;

    if ( ( !responseReceived ) && ( 0U == patternLength ) ) {
        (void)util::strcpy( pattern2Match, pattern, maxStrLength );
        patternLength = ( 0U == n ) ? util::strlen( pattern, maxStrLength ) : n;
        matchedCount = 0U;
        responseReceived = false;
        if ( t > clock::IMMEDIATE ) {
            (void)timeout.set( t );
        }
    }
    else if ( timeout.expired() ) {
        reset();
        retValue = responseStatus::TIMEOUT;
    }
    else if ( responseReceived ) {
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

    if ( ( !responseReceived ) && ( patternLength > 0U ) ) {
        if ( pattern2Match[ matchedCount ] == rxChar ) {
            matchedCount = matchedCount + 1U; /* ++matchedCount; */
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
    return ( maxStrLength > 0U );
}
/*============================================================================*/
