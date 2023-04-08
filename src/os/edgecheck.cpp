#include "edgecheck.hpp"
#include "helper.hpp"

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
    size_t nodeChange = 0u;
    /*cstat -MISRAC++2008-6-5-2*/
    for ( ec::node *n = head ; nullptr != n ; n = n->next ) {
        /*cstat +MISRAC++2008-6-5-2*/
        const ec::pinState v = ( true == reader( n->xPort, n->xPin ) ) ? ec::pinState::ON :
                                                                         ec::pinState::OFF;
        if ( n->prevPinValue != v ) {
            n->status = ec::pinState::UNKNOWN;
            ++nodeChange;
        }
        else {
            n->status = v;
        }
    }

    if ( nodeChange > 0u ) {
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
    /*cstat -MISRAC++2008-6-5-2*/
    for ( ec::node *n = head ; nullptr != n ; n = n->next ) {
        /*cstat +MISRAC++2008-6-5-2*/
        const ec::pinState v = ( true == reader( n->xPort, n->xPin ) ) ? ec::pinState::ON :
                                                                         ec::pinState::OFF;
        
        if ( n->prevPinValue != v ) {
            n->status = ( ec::pinState::ON == v ) ? ec::pinState::RISING_EDGE :
                                                    ec::pinState::FALLING_EDGE;
        }
        n->prevPinValue = v; /*keep the previous level*/
    }
    state = &edgeCheck::stateCheck; /*reload the init state*/
    start = clock::getTick(); /*reload the time*/
}
/*============================================================================*/
ec::nodeReaderFcn_t edgeCheck::getReader( ec::reg rSize ) noexcept
{
    ec::nodeReaderFcn_t f;

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
edgeCheck::edgeCheck( ec::reg rSize, const qOS::clock_t timeDebounce ) noexcept
{
    debounceTime = timeDebounce;
    state = &edgeCheck::stateCheck;
    reader = getReader( rSize );
    start = clock::getTick();
}
/*============================================================================*/
bool edgeCheck::addNode( ec::node& n, void *portAddress, const index_t pinNumber) noexcept
{
    bool retValue = false;

    if ( ( nullptr != portAddress ) && ( pinNumber < 32u ) ) {
        n.xPort = portAddress;
        n.xPin = pinNumber;
        n.next = head;
        n.prevPinValue = ( true == reader( n.xPort, n.xPin ) ) ? ec::pinState::ON : ec::pinState::OFF;
        head = &n;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool edgeCheck::update( void ) noexcept
{
    bool retValue = false;

    if ( nullptr != reader ) {
       (this->*state)();
       retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool ec::node::selectPin( const index_t pinNumber ) noexcept
{
    bool retValue = false;

    if ( pinNumber < 32u ) {
        xPin = pinNumber;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
