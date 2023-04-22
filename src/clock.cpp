#include "include/clock.hpp"

using namespace qOS;

volatile qOS::clock_t clock::sysTick_Epochs = 0u; 
qOS::getTickFcn_t clock::getTick = &internalTick; /* skipcq: CXX-W2009 */
const qOS::time_t clock::NONE = 0uL;
const qOS::time_t clock::IMMEDIATE = 0uL;

/*============================================================================*/
void clock::sysTick( void ) noexcept
{
    ++sysTick_Epochs;
}
/*============================================================================*/
bool clock::timeDeadLineCheck( const qOS::clock_t ti, const qOS::clock_t td ) noexcept
{
    return ( clock::getTick() - ti >= td );
}
/*============================================================================*/
qOS::clock_t clock::internalTick( void ) noexcept
{
    return sysTick_Epochs;
}
/*============================================================================*/
bool clock::setTickProvider( const getTickFcn_t provider ) noexcept
{
    bool retValue = false;

    if ( nullptr != clock::getTick ) {
        if ( nullptr != provider ) {
            clock::getTick = provider;
        }
        else {
            clock::getTick = &internalTick;
        }
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
