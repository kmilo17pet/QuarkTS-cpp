#include "include/input.hpp"

using namespace qOS;


const size_t input::channel::BIT_INVERT = 0U;
const size_t input::channel::BIT_IN_BAND = 0U;
const size_t input::channel::BIT_STEADY_ON = 1U;
const size_t input::channel::BIT_STEADY_OFF = 2U;
const size_t input::channel::BIT_STEADY_BAND = 3U;

/*============================================================================*/
void input::watcher::watchAnalog( channel * const n ) noexcept
{
    if ( ( nullptr != analogReader ) && ( n->riseThreshold > n->fallThreshold ) ) {
        const int current = analogReader( n->xChannel );
        if ( ( input::state::OFF == n->prevState ) && ( current >= n->riseThreshold ) ) {
            n->state = input::state::RISING_EDGE;
            bits::singleSet( n->flags, input::channel::BIT_STEADY_ON );
            bits::singleClear( n->flags, input::channel::BIT_IN_BAND );
            n->tChange = clock::getTick();
            n->prevState = input::state::ON;
            n->invokeEvent( input::event::RISING_EDGE );
        }
        else if ( ( input::state::ON == n->prevState ) && ( current <= n->fallThreshold ) ) {
            n->state = input::state::FALLING_EDGE;
            bits::singleSet( n->flags, input::channel::BIT_STEADY_OFF );
            bits::singleClear( n->flags, input::channel::BIT_IN_BAND );
            n->tChange = clock::getTick();
            n->prevState = input::state::OFF;
            n->invokeEvent( input::event::FALLING_EDGE );
        }
        else if ( !bits::singleRead( n->flags, input::channel::BIT_IN_BAND ) && ( current > n->fallThreshold ) && ( current < n->riseThreshold ) ) {
            bits::singleSet( n->flags, input::channel::BIT_IN_BAND );
            bits::singleSet( n->flags, input::channel::BIT_STEADY_BAND );
            n->tChange = clock::getTick();
            n->invokeEvent( input::event::IN_BAND );
        }
        else {
            const qOS::clock_t tDiff = clock::getTick() - n->tChange;
            if ( bits::singleRead( n->flags, input::channel::BIT_STEADY_ON ) && ( tDiff >= n->tSteadyOn ) ) {
                n->invokeEvent( input::event::STEADY_ON );
                bits::singleClear( n->flags, input::channel::BIT_STEADY_ON );
            }
            if ( bits::singleRead( n->flags, input::channel::BIT_STEADY_OFF ) && ( tDiff >= n->tSteadyOff ) ) {
                n->invokeEvent( input::event::STEADY_OFF );
                bits::singleClear( n->flags, input::channel::BIT_STEADY_OFF );
            }
            if ( bits::singleRead( n->flags, input::channel::BIT_STEADY_BAND ) && ( tDiff >= n->tSteadyBand ) ) {
                n->invokeEvent( input::event::STEADY_IN_BAND );
                bits::singleClear( n->flags, input::channel::BIT_STEADY_BAND );
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
        if ( bits::singleRead( n->flags, input::channel::BIT_INVERT ) ) {
            val = !val; // skipcq: CXX-W2065
        }
        const auto current = static_cast<input::state>( val );

        if ( n->prevState != current ) {
            n->tChange = clock::getTick();
            if ( input::state::ON == current ) {
                n->state = input::state::RISING_EDGE;
                bits::singleSet( n->flags, input::channel::BIT_STEADY_ON );
                n->invokeEvent( input::event::RISING_EDGE );
            }
            else {
                n->state = input::state::FALLING_EDGE;
                bits::singleSet( n->flags, input::channel::BIT_STEADY_OFF );
                n->invokeEvent( input::event::FALLING_EDGE );
            }
        }
        else {
            const qOS::clock_t tDiff = clock::getTick() - n->tChange;
            if ( bits::singleRead( n->flags, input::channel::BIT_STEADY_ON ) && ( current == input::state::ON ) && ( tDiff >= n->tSteadyOn ) ) {
                n->invokeEvent( input::event::STEADY_ON );
                bits::singleClear( n->flags, input::channel::BIT_STEADY_ON );
            }
            if ( bits::singleRead( n->flags, input::channel::BIT_STEADY_OFF ) && ( current == input::state::OFF ) && ( tDiff >= n->tSteadyOff ) ) {
                n->invokeEvent( input::event::STEADY_OFF );
                bits::singleClear( n->flags, input::channel::BIT_STEADY_OFF );
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
bool input::channel::registerEvent( const input::event e, const input::eventCallback_t &c, const qOS::duration_t t ) noexcept
{
    bool retVal = true;
    const auto cbIndex = static_cast<int>( e );

    switch ( e ) {
        case input::event::RISING_EDGE: case input::event::FALLING_EDGE: case input::event::IN_BAND: // skipcq: CXX-C1001
            cb[ cbIndex ] = c;
            break;
        case input::event::STEADY_ON:
            cb[ cbIndex ] = c;
            tSteadyOn = static_cast<qOS::clock_t>( t );
            break;
        case input::event::STEADY_OFF:
            cb[ cbIndex ] = c;
            tSteadyOff = static_cast<qOS::clock_t>( t );
            break;
        case input::event::STEADY_IN_BAND:
            cb[ cbIndex ] = c;
            tSteadyBand = static_cast<qOS::clock_t>( t );
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
        if ( bits::singleRead( n.flags, input::channel::BIT_INVERT ) ) {
            val = !val; // skipcq: CXX-W2065
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
bool input::watcher::registerEvent( const event e, const eventCallback_t &c, const qOS::duration_t t ) noexcept
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
