#include "timer.hpp"

using namespace qOS;

const bool timer::ARMED = true;
const bool timer::DISARMED = false;
const qOS::clock_t timer::DISARM_VALUE = 0uL ;
const qOS::clock_t timer::REMAINING_IN_DISARMED_STATE = 0xFFFFFFFFuL;

/*============================================================================*/
timer::timer()
{
    this->disarm();
}
/*============================================================================*/
void timer::reload( void ) 
{
    tStart = clock::getTick();
}
/*============================================================================*/
bool timer::set( qOS::time_t tTime )
{
    bool retValue = false;
    
    if ( tTime > static_cast<qOS::time_t>( 0 ) ) {
        this->reload();
        tv = clock::convert2Clock( tTime );
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
timer& timer::operator=( qOS::time_t tTime )
{
    if( tTime > static_cast<qOS::time_t>( 0 ) ) {
        set( tTime );
    }
    else {
        disarm();
    }
    return *this;
}
/*============================================================================*/
timer& timer::operator=( bool en )
{
    ( en )? reload() : disarm();
    return *this;
}
/*============================================================================*/
bool timer::operator()( void )
{
    return expired();
}
/*============================================================================*/
void timer::disarm( void )
{
    this->tv = timer::DISARM_VALUE;
    this->tStart = timer::DISARM_VALUE;
}
/*============================================================================*/
bool timer::freeRun( time_t tTime )
{
    bool retValue = false;

    if ( timer::ARMED == this->status() ) {
        if ( this->expired() ) {
            this->reload();
            retValue = true;
        }
    }
    else {
        (void)this->set( tTime );
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
qOS::clock_t timer::elapsed( void ) const
{
    return ( timer::ARMED == this->status() ) ? clock::getTick() - this->tStart : 0u; 
}
/*============================================================================*/
qOS::clock_t timer::remaining( void ) const
{
    return ( timer::DISARM_VALUE != this->status() ) ? this->tv - this->elapsed() 
                                                     : timer::REMAINING_IN_DISARMED_STATE; 
}
/*============================================================================*/
bool timer::expired( void ) const
{
    bool retValue = false;
    
    if ( timer::ARMED == this->status() ) {
        retValue = deadLineCheck();
    }
    
    return retValue;
}
/*============================================================================*/
bool timer::status( void ) const
{
    return ( this->tv != 0u );
}
/*============================================================================*/
bool timer::deadLineCheck( void ) const
{
    return ( clock::getTick() - this->tStart >= this->tv );
}
/*============================================================================*/
qOS::clock_t timer::getInterval( void ) const
{
    return this->tv;
}
/*============================================================================*/