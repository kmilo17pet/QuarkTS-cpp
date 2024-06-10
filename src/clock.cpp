#include "include/clock.hpp"

using namespace qOS;

volatile qOS::clock_t clock::sysTick_Epochs = 0U; // skipcq: CXX-W2009
qOS::getTickFcn_t clock::getTick = &internalTick; // skipcq: CXX-W2009
/*cstat -MISRAC++2008-2-13-3*/
const qOS::duration_t clock::NONE = 0_ms;
const qOS::duration_t clock::IMMEDIATE = 0_ms;
/*cstat +MISRAC++2008-2-13-3*/
/*============================================================================*/
void clock::sysTick( void ) noexcept
{
    sysTick_Epochs = sysTick_Epochs + 1U; /* ++sysTick_Epochs */
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
