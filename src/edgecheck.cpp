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
void edgeCheck::stateCheck( void )
{
    size_t nodeChange = 0U;

    for ( auto i = nodes.begin(); i.until() ; i++ ) {
        ec::inNode * const n = i.get<ec::inNode*>();

        const ec::pinState v = readPin( n );
        if ( n->prevPinValue != v ) {
            n->status = ec::pinState::UNKNOWN;
            ++nodeChange;
        }
        else {
            n->status = v;
        }
    }

    if ( nodeChange > 0U ) {
        state = &edgeCheck::stateWait;
    }
}
/*============================================================================*/
void edgeCheck::stateWait( void )
{
    if ( clock::timeDeadLineCheck( start, debounceTime ) ) {
        state = &edgeCheck::stateUpdate;
    }
}
/*============================================================================*/
void edgeCheck::stateUpdate( void )
{
    for ( auto i = nodes.begin(); i.until() ; i++ ) {
        ec::inNode * const n = i.get<ec::inNode*>();
        const ec::pinState v = readPin( n );

        if ( n->prevPinValue != v ) {
            n->status = ( ec::pinState::ON == v ) ? ec::pinState::RISING_EDGE :
                                                    ec::pinState::FALLING_EDGE;
        }
        n->prevPinValue = v;
    }
    state = &edgeCheck::stateCheck;
    start = clock::getTick();
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
edgeCheck::edgeCheck( ec::nodePinReaderFcn_t bitReader, const qOS::clock_t timeDebounce ) noexcept : debounceTime( timeDebounce )
{
    state = &edgeCheck::stateCheck;
    pinReader = bitReader;
    start = clock::getTick();
}
/*============================================================================*/
edgeCheck::edgeCheck( ec::reg rSize, const qOS::clock_t timeDebounce ) noexcept : debounceTime( timeDebounce )
{
    state = &edgeCheck::stateCheck;
    portReader = getReader( rSize );
    start = clock::getTick();
}
/*============================================================================*/
bool edgeCheck::add( ec::inNode& n, const index_t pinNumber, void *portAddress ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != portAddress ) && ( pinNumber < 32U ) ) {
        n.xPort = portAddress;
        n.xPin = pinNumber;
        n.prevPinValue = readPin( &n );
        retValue = nodes.insert( &n );
    }

    return retValue;
}
/*============================================================================*/
bool edgeCheck::update( void ) noexcept
{
    bool retValue = false;

    if ( nullptr != portReader ) {
       (this->*state)();
       retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool ec::inNode::selectPin( const index_t pinNumber ) noexcept
{
    bool retValue = false;

    if ( pinNumber < 32U ) {
        xPin = pinNumber;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
