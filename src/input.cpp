#include "include/input.hpp"
#include "include/helper.hpp"

using namespace qOS;

/*============================================================================*/
void input::watcher::watchAnalog( channel * const n ) noexcept
{
    int current = 0;

    if ( nullptr != analogReader ) {
        current = analogReader( n->xChannel );
    }

    if ( ( input::state::OFF == n->prevState ) && ( current >= n->riseThreshold ) ) {
        n->state = input::state::RISING_EDGE;
        n->bSteadyOn = true;
        n->tChange = clock::getTick();
        n->prevState = input::state::ON;
        if ( nullptr != n->risingCB ) {
            n->risingCB( n->xChannel, input::event::RISING_EDGE );
        }
    }
    else if ( ( input::state::ON == n->prevState ) && ( current <= n->fallThreshold ) ) {
        n->state = input::state::FALLING_EDGE;
        n->bSteadyOff = true;
        n->tChange = clock::getTick();
        n->prevState = input::state::OFF;
        if ( nullptr != n->fallingCB ) {
            n->fallingCB( n->xChannel, input::event::FALLING_EDGE );
        }
    }
    else {
        const qOS::clock_t tDiff = clock::getTick() - n->tChange;
        if ( n->bSteadyOn && ( nullptr != n->steadyOnCB ) && ( tDiff >= n->tSteadyOn ) ) {
            n->steadyOnCB( n->xChannel, input::event::STEADY_ON );
            n->bSteadyOn = false;
        }
        if ( n->bSteadyOff && ( nullptr != n->steadyOffCB ) && ( tDiff >= n->tSteadyOff ) ) {
            n->steadyOffCB( n->xChannel, input::event::STEADY_OFF );
            n->bSteadyOff = false;
        }
    }
}
/*============================================================================*/
void input::watcher::watchDigital( channel * const n ) noexcept
{
    auto current = read( n );

    if ( n->prevState != current ) {
        n->tChange = clock::getTick();
        if ( input::state::ON == current ) {
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
        if ( n->bSteadyOn && ( nullptr != n->steadyOnCB ) && ( current == input::state::ON ) && ( tDiff >= n->tSteadyOn ) ) {
            n->steadyOnCB( n->xChannel, input::event::STEADY_ON );
            n->bSteadyOn = false;
        }
        if ( n->bSteadyOff && ( nullptr != n->steadyOffCB ) && ( current == input::state::OFF ) && ( tDiff >= n->tSteadyOff ) ) {
            n->steadyOffCB( n->xChannel, input::event::STEADY_OFF );
            n->bSteadyOff = false;
        }
    }
    n->prevState = current;

}
/*============================================================================*/
bool input::watcher::watch( void ) noexcept
{
    auto debouncePassed = waitDebounce.freeRun( debounceTime );

    for ( auto i = nodes.begin(); i.untilEnd() ; i++ ) {
        input::channel * const n = i.get<input::channel*>();
        if ( input::type::ANALOG == n->channelType ) {
            watchAnalog( n );
        }
        else if ( debouncePassed ) {
            watchDigital( n );
        }
    }
    return true;

}
/*============================================================================*/
bool input::channel::registerEvent( input::event e, input::eventCallback_t c, qOS::clock_t t ) noexcept
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
