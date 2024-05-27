#include "include/input.hpp"

using namespace qOS;

/*============================================================================*/
void input::watcher::watchAnalog( channel& c ) noexcept
{
    if ( ( nullptr != analogReader ) && ( ( c.riseThreshold - c.fallThreshold ) > c.hysteresis ) ) {
        const int val = analogReader( c.xChannel );
        c.channelState( c, val );
    }
    else {
        exceptionEvent( c );
    }
}
/*============================================================================*/
void input::watcher::watchDigital( channel& c ) noexcept
{
    if ( nullptr != digitalReader ) {
        auto val = digitalReader( c.xChannel );
        if ( c.negate ) {
            val = !val; // skipcq: CXX-W2065
        }
        c.channelState( c, val );
    }
    else {
        exceptionEvent( c );
    }
}
/*============================================================================*/
void input::channel::digitalFallingEdgeState( channel& c, int value )
{
    if ( 0 != value ) {
        c.channelState = &input::channel::digitalRisingEdgeState;
        c.invokeEvent( input::event::RISING_EDGE );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
    if ( ( clock::getTick() - c.tChange ) > c.tSteadyOff ) {
        c.channelState = &input::channel::digitalSteadyInLowState;
        c.invokeEvent( input::event::STEADY_IN_LOW );
        c.tChange = clock::getTick();
    }
}
/*============================================================================*/
void input::channel::digitalSteadyInLowState( channel& c, int value )
{
    if ( 0 != value ) {
        c.channelState = &input::channel::digitalRisingEdgeState;
        c.invokeEvent( input::event::RISING_EDGE );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
}
/*============================================================================*/
void input::channel::digitalRisingEdgeState( channel& c, int value )
{
    if ( 0 == value ) {
        c.channelState = &input::channel::digitalFallingEdgeState;
        c.invokeEvent( input::event::FALLING_EDGE );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
    if ( ( clock::getTick() - c.tChange ) > c.tSteadyOn ) {
        c.channelState = &input::channel::digitalSteadyInHighState;
        c.invokeEvent( input::event::STEADY_IN_HIGH );
        c.tChange = clock::getTick();
    }
}
/*============================================================================*/
void input::channel::digitalSteadyInHighState( channel& c, int value )
{
    if ( 0 == value ) {
        c.channelState = &input::channel::digitalFallingEdgeState;
        c.invokeEvent( input::event::FALLING_EDGE );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
}
/*============================================================================*/
void input::channel::analogFallingEdgeState( channel& c, int value )
{
    if ( value > c.riseThreshold ) {
        c.channelState = &input::channel::analogRisingEdgeState;
        c.invokeEvent( input::event::RISING_EDGE );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
    else if ( value > ( c.fallThreshold + c.hysteresis ) ) {
        c.channelState = &input::channel::analogInBandState;
        c.invokeEvent( input::event::IN_BAND );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
    else {
        /*Nothing to do here*/
    }

    if ( ( clock::getTick() - c.tChange ) > c.tSteadyOff ) {
        c.channelState = &input::channel::analogSteadyInLowState;
        c.invokeEvent( input::event::STEADY_IN_LOW );
        c.tChange = clock::getTick();
    }
}
/*============================================================================*/
void input::channel::analogRisingEdgeState( channel& c, int value )
{
    if ( value < c.fallThreshold ) {
        c.channelState = &input::channel::analogFallingEdgeState;
        c.invokeEvent( input::event::FALLING_EDGE );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
    else if ( value < ( c.riseThreshold - c.hysteresis ) ) {
        c.channelState = &input::channel::analogInBandState;
        c.invokeEvent( input::event::IN_BAND );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
    else {
        /*Nothing to do here*/
    }

    if ( ( clock::getTick() - c.tChange ) > c.tSteadyOn ) {
        c.channelState = &input::channel::analogSteadyInHighState;
        c.invokeEvent( input::event::STEADY_IN_HIGH );
        c.tChange = clock::getTick();
    }
}
/*============================================================================*/
void input::channel::analogInBandState( channel& c, int value )
{
    if ( value > c.riseThreshold ) {
        c.channelState = &input::channel::analogRisingEdgeState;
        c.invokeEvent( input::event::RISING_EDGE );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
    else if ( value < c.fallThreshold ) {
        c.channelState = &input::channel::analogFallingEdgeState;
        c.invokeEvent( input::event::FALLING_EDGE );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
    else {
        /*Nothing to do here*/
    }

    if ( ( clock::getTick() - c.tChange ) > c.tSteadyBand ) {
        c.channelState = &input::channel::analogSteadyInBandState;
        c.invokeEvent( input::event::STEADY_IN_BAND );
        c.tChange = clock::getTick();
    }
}
/*============================================================================*/
void input::channel::analogSteadyInHighState( channel& c, int value )
{
    if ( value < c.fallThreshold ) {
        c.channelState = &input::channel::analogFallingEdgeState;
        c.invokeEvent( input::event::FALLING_EDGE );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
    else if ( value < ( c.riseThreshold - c.hysteresis ) ) {
        c.channelState = &input::channel::analogInBandState;
        c.invokeEvent( input::event::IN_BAND );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
    else {
        /*Nothing to do here*/
    }
}
/*============================================================================*/
void input::channel::analogSteadyInLowState( channel& c, int value )
{
    if ( value > c.riseThreshold ) {
        c.channelState = &input::channel::analogRisingEdgeState;
        c.invokeEvent( input::event::RISING_EDGE );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
    else if ( value > ( c.fallThreshold + c.hysteresis ) ) {
        c.channelState = &input::channel::analogInBandState;
        c.invokeEvent( input::event::IN_BAND );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
    else {
        /*Nothing to do here*/
    }
}
/*============================================================================*/
void input::channel::analogSteadyInBandState( channel& c, int value )
{
    if ( value > c.riseThreshold ) {
        c.channelState = &input::channel::analogRisingEdgeState;
        c.invokeEvent( input::event::RISING_EDGE );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
    else if ( value < c.fallThreshold ) {
        c.channelState = &input::channel::analogFallingEdgeState;
        c.invokeEvent( input::event::FALLING_EDGE );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
    else {
        /*Nothing to do here*/
    }
}
/*============================================================================*/
bool input::watcher::watch( void ) noexcept
{
    const auto debouncePassed = waitDebounce.freeRun( debounceTime );

    for ( auto i = nodes.begin(); i.untilEnd() ; i++ ) {
        input::channel& c = *i.get<input::channel*>();

        if ( ( input::type::ANALOG == c.channelType ) ) {
            watchAnalog( c );
        }
        else if ( debouncePassed ) {
            watchDigital( c );
        }
        else {
            /*nothing to do here*/
        }
    }
    return true;
}
/*============================================================================*/
bool input::channel::registerEvent( const input::event e, const input::eventCallback_t &f, const qOS::duration_t t ) noexcept
{
    bool retVal = false;
    const auto cbIndex = static_cast<int>( e );

    if ( e < input::event::MAX_EVENTS ) {
        cb[ cbIndex ] = f;
        switch ( e ) {
            case input::event::STEADY_IN_HIGH:
                tSteadyOn = static_cast<qOS::clock_t>( t );
                break;
            case input::event::STEADY_IN_LOW:
                tSteadyOff = static_cast<qOS::clock_t>( t );
                break;
            case input::event::STEADY_IN_BAND:
                tSteadyBand = static_cast<qOS::clock_t>( t );
                break;
            default:
                break;
        }
        retVal = true;
    }

    return retVal;
}
/*============================================================================*/
bool input::watcher::add( channel& c ) noexcept
{
    if ( input::type::DIGITAL == c.channelType ) {
        auto val = ( nullptr != digitalReader ) ? digitalReader( c.xChannel ) : -1;
        if ( c.negate ) {
            val = !val; // skipcq: CXX-W2065
        }
        c.channelState = ( 0 == val ) ? &input::channel::digitalFallingEdgeState
                                      : &input::channel::digitalRisingEdgeState;
    }
    else {
        const auto val = ( nullptr != analogReader ) ? analogReader( c.xChannel ) : -1;
        if ( val > c.riseThreshold ) {
            c.channelState = &input::channel::analogRisingEdgeState;
        }
        else if ( val < c.fallThreshold ) {
            c.channelState = &input::channel::analogFallingEdgeState;
        }
        else {
            c.channelState = &input::channel::analogInBandState;
        }
    }
    c.tChange = clock::getTick();

    return nodes.insert( &c );
}
/*============================================================================*/
bool input::watcher::registerEvent( const event e, const eventCallback_t &f, const qOS::duration_t t ) noexcept
{
    bool retValue = true;

    if ( input::event::EXCEPTION == e ) {
        exception = f;
    }
    else {
        for ( auto i = nodes.begin(); i.untilEnd() ; i++ ) {
            input::channel * const n = i.get<input::channel*>();
            retValue &= n->registerEvent( e, f, t );
        }
    }

    return retValue;
}
/*============================================================================*/
