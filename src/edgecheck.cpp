#include "include/edgecheck.hpp"
#include "include/helper.hpp"

using namespace qOS;


static bool read8bit( void *addr, index_t pinNumber );
static bool read16bit( void *addr, index_t pinNumber );
static bool read32bit( void *addr, index_t pinNumber );

/*============================================================================*/
static bool read8bit( void *addr, index_t pinNumber )
{
    /*cstat -CERT-EXP36-C_b*/
    uint8_t * const reg = static_cast<uint8_t*>( addr );
    /*cstat +CERT-EXP36-C_b*/
    return bits::singleRead( reg[ 0 ], pinNumber );
}
/*============================================================================*/
static bool read16bit( void *addr, index_t pinNumber )
{
    /*cstat -CERT-EXP36-C_b*/
    uint16_t * const reg = static_cast<uint16_t*>( addr );
    /*cstat +CERT-EXP36-C_b*/
    return bits::singleRead( reg[ 0 ], pinNumber );
}
/*============================================================================*/
static bool read32bit( void *addr, index_t pinNumber )
{
    /*cstat -CERT-EXP36-C_b*/
    uint32_t * const reg = static_cast<uint32_t*>( addr );
    /*cstat +CERT-EXP36-C_b*/
    return bits::singleRead( reg[ 0 ], pinNumber );
}
/*============================================================================*/
ec::nodePortReaderFcn_t edgeCheck::getReader( ec::reg rSize ) noexcept
{
    ec::nodePortReaderFcn_t f;

    if ( ec::reg::SIZE_8_BIT == rSize ) {
        f = &read8bit;
    }
    else if ( ec::reg::SIZE_16_BIT == rSize ) {
        f = &read16bit;
    }
    else {
        f = &read32bit;
    }

    return f;
}
/*============================================================================*/
bool edgeCheck::update( void ) noexcept
{
    bool retValue = false;

    for ( auto i = nodes.begin(); i.untilEnd() ; i++ ) {
        ec::inNode * const n = i.get<ec::inNode*>();
        n->current = readPin( n );
        n->state = ec::pinState::UNKNOWN;
    }
    if ( waitDebounce.freeRun( debounceTime ) ) {
        for ( auto i = nodes.begin(); i.untilEnd() ; i++ ) {
            ec::inNode * const n = i.get<ec::inNode*>();
            const auto v = readPin( n );

            if ( n->prevPinValue != v ) {
                n->state = ( ec::pinState::ON == v ) ? ec::pinState::RISING_EDGE :
                                                       ec::pinState::FALLING_EDGE;
            }
            n->prevPinValue = v;
        }
        retValue = true;
    }

    return retValue;
}

