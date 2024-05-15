#include "include/input.hpp"
#include "include/helper.hpp"

using namespace qOS;

/*============================================================================*/
bool input::observer::update( void ) noexcept
{
    bool retValue = false;

    for ( auto i = nodes.begin(); i.untilEnd() ; i++ ) {
        input::channel * const n = i.get<input::channel*>();
        n->current = readPin( n );
        n->state = input::state::UNKNOWN;
    }
    if ( waitDebounce.freeRun( debounceTime ) ) {
        for ( auto i = nodes.begin(); i.untilEnd() ; i++ ) {
            input::channel * const n = i.get<input::channel*>();
            const auto v = readPin( n );

            if ( n->prevPinValue != v ) {
                n->tChange = clock::getTick();
                if ( input::state::ON == v ) {
                    n->state = input::state::RISING_EDGE;
                    n->bSteadyOn = true;
                    if ( nullptr != n->risingCB ) {
                        n->risingCB( n->xChannel, input::event::RISING_EDGE );
                    }
                }
                else {
                    n->state = input::state::FALLING_EDGE;
                    n->bSteadyOff = true;
                    if ( nullptr != n->fallingCB ) {
                        n->fallingCB( n->xChannel, input::event::FALLING_EDGE );
                    }
                }
            }
            else {
                const qOS::clock_t tDiff = clock::getTick() - n->tChange;
                if ( n->bSteadyOn && ( nullptr != n->steadyOnCB ) && ( n->current == input::state::ON ) && ( tDiff >= n->tSteadyOn ) ) {
                    n->steadyOnCB( n->xChannel, input::event::STEADY_ON );
                    n->bSteadyOn = false;
                }
                if ( n->bSteadyOff && ( nullptr != n->steadyOffCB ) && ( n->current == input::state::OFF ) && ( tDiff >= n->tSteadyOff ) ) {
                    n->steadyOffCB( n->xChannel, input::event::STEADY_OFF );
                    n->bSteadyOff = false;
                }
            }
            n->prevPinValue = v;
        }
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool input::channel::setCallback( input::event e, input::eventCallback_t c, qOS::clock_t t )
{
    bool retVal = true;

    switch ( e ) {
        case input::event::RISING_EDGE:
            risingCB = c;
            break;
        case input::event::FALLING_EDGE:
            fallingCB = c;
            break;
        case input::event::STEADY_ON:
            steadyOnCB = c;
            tSteadyOn = t;
            break;
        case input::event::STEADY_OFF:
            steadyOffCB = c;
            tSteadyOff = t;
            break;
        default:
            retVal = false;
            break;
    }

    return retVal;
}
