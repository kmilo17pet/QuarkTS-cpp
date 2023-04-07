#include "response.hpp"
#include "util.hpp"

using namespace qOS;

/*============================================================================*/
bool response::setup( char *xLocBuff, const std::size_t nMax ) noexcept
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
responseStatus response::received( const char *pattern, const std::size_t n, qOS::time_t t ) noexcept
{
    responseStatus retValue = responseStatus::MISSING;

    if ( ( false == responseReceived ) && ( 0u == patternLength ) ) {
        (void)util::strcpy( pattern2Match, pattern, maxStrLength );
        patternLength = ( 0u == n ) ? util::strlen( pattern, maxStrLength ) : n;
        matchedCount = 0u; /*reinitialize the chars match count*/
        responseReceived = false; /*clear the ready flag*/
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
        /*check if the received char match with the expected*/
        if ( pattern2Match[ matchedCount ] == rxChar ) {
            /*move to the next char in the expected buffer*/
            ++matchedCount;
            if ( matchedCount == patternLength ) {
                responseReceived = true;
                /*if all the requested chars match, set the ready flag */
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
