#include "include/input.hpp"
#include "include/helper.hpp"

using namespace qOS;

/*============================================================================*/
void input::watcher::watchAnalog( channel * const n ) noexcept
{
    if ( ( nullptr != analogReader ) && ( n->riseThreshold > n->fallThreshold ) ) {
        const int current = analogReader( n->xChannel );
        if ( ( input::state::OFF == n->prevState ) && ( current >= n->riseThreshold ) ) {
            n->state = input::state::RISING_EDGE;
            n->bSteadyOn = true;
            n->tChange = clock::getTick();
            n->prevState = input::state::ON;
            if ( nullptr != n->risingCB ) {
                n->risingCB( *n, input::event::RISING_EDGE );
            }
        }
        else if ( ( input::state::ON == n->prevState ) && ( current <= n->fallThreshold ) ) {
            n->state = input::state::FALLING_EDGE;
            n->bSteadyOff = true;
            n->tChange = clock::getTick();
            n->prevState = input::state::OFF;
            if ( nullptr != n->fallingCB ) {
                n->fallingCB( *n, input::event::FALLING_EDGE );
            }
        }
        else {
            const qOS::clock_t tDiff = clock::getTick() - n->tChange;
            if ( n->bSteadyOn && ( nullptr != n->steadyOnCB ) && ( tDiff >= n->tSteadyOn ) ) {
                n->steadyOnCB( *n, input::event::STEADY_ON );
                n->bSteadyOn = false;
            }
            if ( n->bSteadyOff && ( nullptr != n->steadyOffCB ) && ( tDiff >= n->tSteadyOff ) ) {
                n->steadyOffCB( *n, input::event::STEADY_OFF );
                n->bSteadyOff = false;
            }
        }
    }
    else {
        exceptionEvent( n );
    }
}
/*============================================================================*/
void input::watcher::watchDigital( channel * const n ) noexcept
{
    if ( nullptr != digitalReader ) {
        auto val = digitalReader( n->xChannel );
        if ( n->negValue ) {
            val = !val;
        }
        const auto current = static_cast<input::state>( val );

        if ( n->prevState != current ) {
            n->tChange = clock::getTick();
            if ( input::state::ON == current ) {
                n->state = input::state::RISING_EDGE;
                n->bSteadyOn = true;
                if ( nullptr != n->risingCB ) {
                    n->risingCB( *n, input::event::RISING_EDGE );
                }
            }
            else {
                n->state = input::state::FALLING_EDGE;
                n->bSteadyOff = true;
                if ( nullptr != n->fallingCB ) {
                    n->fallingCB( *n, input::event::FALLING_EDGE );
                }
            }
        }
        else {
            const qOS::clock_t tDiff = clock::getTick() - n->tChange;
            if ( n->bSteadyOn && ( nullptr != n->steadyOnCB ) && ( current == input::state::ON ) && ( tDiff >= n->tSteadyOn ) ) {
                n->steadyOnCB( *n, input::event::STEADY_ON );
                n->bSteadyOn = false;
            }
            if ( n->bSteadyOff && ( nullptr != n->steadyOffCB ) && ( current == input::state::OFF ) && ( tDiff >= n->tSteadyOff ) ) {
                n->steadyOffCB( *n, input::event::STEADY_OFF );
                n->bSteadyOff = false;
            }
        }
        n->prevState = current;
    }
    else {
        exceptionEvent( n );
    }
}
/*============================================================================*/
bool input::watcher::watch( void ) noexcept
{
    const auto debouncePassed = waitDebounce.freeRun( debounceTime );

    for ( auto i = nodes.begin(); i.untilEnd() ; i++ ) {
        input::channel * const n = i.get<input::channel*>();
        if ( ( input::type::ANALOG == n->channelType ) ) {
            watchAnalog( n );
        }
        else if ( debouncePassed ) {
            watchDigital( n );
        }
        else {
            /*nothing to do here*/
        }
    }
    return true;
}
/*============================================================================*/
bool input::channel::registerEvent( input::event e, input::eventCallback_t c, qOS::duration_t t ) noexcept
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
            tSteadyOn = static_cast<qOS::clock_t>( t );
            break;
        case input::event::STEADY_OFF:
            steadyOffCB = c;
            tSteadyOff = static_cast<qOS::clock_t>( t );
            break;
        default:
            retVal = false;
            break;
    }

    return retVal;
}
/*============================================================================*/
bool input::watcher::add( channel& n ) noexcept
{
    if ( input::type::DIGITAL == n.channelType ) {
        auto val = ( nullptr != digitalReader ) ? digitalReader( n.xChannel ) : -1;
        if ( n.negValue ) {
            val = !val;
        }
        n.prevState = static_cast<input::state>( val );
    }
    else {
        n.prevState = input::state::OFF;
    }
    n.tChange = clock::getTick();

    return nodes.insert( &n );
}
/*============================================================================*/
bool input::watcher::registerEvent( event e, eventCallback_t c, qOS::duration_t t ) noexcept
{
    bool retValue = true;

    if ( input::event::EXCEPTION == e ) {
        exception = c;
    }
    else {
        for ( auto i = nodes.begin(); i.untilEnd() ; i++ ) {
            input::channel * const n = i.get<input::channel*>();
            retValue &= n->registerEvent( e, c, t );
        }
    }

    return retValue;
}
