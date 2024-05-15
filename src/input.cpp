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
                    n->bPressed = true;
                    if ( nullptr != n->risingCB ) {
                        n->risingCB( n->xChannel, input::event::RISING_EDGE );
                    }
                }
                else {
                    n->state = input::state::FALLING_EDGE;
                    n->bReleased = true;
                    if ( nullptr != n->fallingCB ) {
                        n->fallingCB( n->xChannel, input::event::FALLING_EDGE );
                    }
                }
            }
            else {
                const qOS::clock_t tDiff = clock::getTick() - n->tChange;
                if ( n->bPressed && ( nullptr != n->pressedCB ) && ( n->current == input::state::ON ) && ( tDiff >= n->tPressed ) ) {
                    n->pressedCB( n->xChannel, input::event::PRESSED );
                    n->bPressed = false;
                }
                if ( n->bReleased && ( nullptr != n->releasedCB ) && ( n->current == input::state::OFF ) && ( tDiff >= n->tReleased ) ) {
                    n->releasedCB( n->xChannel, input::event::RELEASED );
                    n->bReleased = false;
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
        case input::event::PRESSED:
            pressedCB = c;
            tPressed = t;
            break;
        case input::event::RELEASED:
            releasedCB = c;
            tReleased = t;
            break;
        default:
            retVal = false;
            break;
    }

    return retVal;
}