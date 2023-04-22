#include "include/timer.hpp"

using namespace qOS;

const bool timer::ARMED = true;
const bool timer::DISARMED = false;
const qOS::clock_t timer::DISARM_VALUE = 0uL ;
const qOS::clock_t timer::REMAINING_IN_DISARMED_STATE = 0xFFFFFFFFuL;

/*============================================================================*/
timer::timer()
{
    disarm();
}
/*============================================================================*/
void timer::reload( void ) noexcept
{
    tStart = clock::getTick();
}
/*============================================================================*/
bool timer::set( qOS::time_t tTime ) noexcept
{
    bool retValue = false;
    /*cstat -CERT-FLP36-C*/
    if ( tTime > static_cast<qOS::time_t>( 0 ) ) {
        reload();
        tv = static_cast<clock_t>( tTime );
        retValue = true;
    }
    /*cstat +CERT-FLP36-C*/
    return retValue;
}
/*============================================================================*/
timer& timer::operator=( qOS::time_t tTime )
{
    /*cstat -CERT-FLP36-C*/
    if( tTime > static_cast<qOS::time_t>( 0 ) ) {
        (void)set( tTime );
    }
    /*cstat +CERT-FLP36-C*/
    else {
        disarm();
    }
    return *this;
}
/*============================================================================*/
timer& timer::operator=( bool en )
{
    ( en ) ? reload() : disarm();
    return *this;
}
/*============================================================================*/
bool timer::operator()( void )
{
    return expired();
}
/*============================================================================*/
void timer::disarm( void ) noexcept
{
    tv = timer::DISARM_VALUE;
    tStart = timer::DISARM_VALUE;
}
/*============================================================================*/
bool timer::freeRun( qOS::time_t tTime ) noexcept
{
    bool retValue = false;

    if ( timer::ARMED == status() ) {
        if ( expired() ) {
            reload();
            retValue = true;
        }
    }
    else {
        (void)set( tTime );
    }

    return retValue;
}
/*============================================================================*/
bool timer::operator()( qOS::time_t tTime )
{
    return set( tTime );
}
/*============================================================================*/
void timer::operator()( bool en )
{
    ( en )? reload() : disarm();
}
/*============================================================================*/
qOS::clock_t timer::elapsed( void ) const noexcept
{
    return ( timer::ARMED == status() ) ? clock::getTick() - tStart : 0u; 
}
/*============================================================================*/
qOS::clock_t timer::remaining( void ) const noexcept
{
    return ( timer::DISARM_VALUE != status() ) ? tv - elapsed() : timer::REMAINING_IN_DISARMED_STATE; 
}
/*============================================================================*/
bool timer::expired( void ) const noexcept
{
    bool retValue = false;
    
    if ( timer::ARMED == status() ) {
        retValue = deadLineCheck();
    }
    
    return retValue;
}
/*============================================================================*/
bool timer::status( void ) const noexcept
{
    return ( tv != 0u );
}
/*============================================================================*/
bool timer::deadLineCheck( void ) const noexcept
{
    return ( ( clock::getTick() - tStart ) >= tv );
}
/*============================================================================*/
qOS::clock_t timer::getInterval( void ) const noexcept
{
    return tv;
}
/*============================================================================*/
