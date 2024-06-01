#include "include/input.hpp"

using namespace qOS;

/*============================================================================*/
void input::watcher::watchAnalog( channel& c ) noexcept
{
    if ( ( nullptr != analogReader ) && ( ( c.riseThreshold - c.fallThreshold ) > c.hysteresis ) ) {
        c.value = ( c.isShared() ) ? c.ptrValue[ 0 ] : analogReader( c.number );
        c.channelState( c );
    }
    else {
        exceptionEvent( c );
    }
}
/*============================================================================*/
void input::watcher::watchDigital( channel& c ) noexcept
{
    if ( nullptr != digitalReader ) {
        c.value = digitalReader( c.number );
        if ( c.negate ) {
            c.value = !c.value; // skipcq: CXX-W2065
        }
        c.channelState( c );
    }
    else {
        exceptionEvent( c );
    }
}
/*============================================================================*/
void input::channel::digitalFallingEdgeState( channel& c )
{
    const qOS::clock_t CURRENT_TIME = clock::getTick();

    if ( 0 != c.value ) {
        c.channelState = &input::channel::digitalRisingEdgeState;
        c.invokeEvent( input::event::RISING_EDGE );
        c.invokeEvent( input::event::ON_CHANGE );
        if ( ( CURRENT_TIME - c.tChange ) > c.tMultiPressInterval ) {
            c.multiPressCount = 0U;
        }
        c.tChange = CURRENT_TIME;
    }
    if ( ( CURRENT_TIME - c.tChange ) > c.tSteadyOff ) {
        c.channelState = &input::channel::digitalSteadyInLowState;
        c.invokeEvent( input::event::STEADY_IN_LOW );
        c.tChange = CURRENT_TIME;
    }
}
/*============================================================================*/
void input::channel::digitalSteadyInLowState( channel& c )
{
    if ( 0 != c.value ) {
        c.channelState = &input::channel::digitalRisingEdgeState;
        c.invokeEvent( input::event::RISING_EDGE );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
}
/*============================================================================*/
void input::channel::digitalRisingEdgeState( channel& c )
{
    const qOS::clock_t CURRENT_TIME = clock::getTick();

    if ( 0 == c.value ) {
        c.channelState = &input::channel::digitalFallingEdgeState;
        c.invokeEvent( input::event::FALLING_EDGE );
        c.invokeEvent( input::event::ON_CHANGE );
        if ( ( CURRENT_TIME - c.tChange ) <= c.tMultiPressInterval ) {
            ++c.multiPressCount;
        }
        c.tChange = CURRENT_TIME;

        switch ( c.multiPressCount ) {
            case 0 : case 1: break;
            case 2:
                c.invokeEvent( input::event::MULTI_PRESS_DOUBLE );
                break;
            case 3:
                c.invokeEvent( input::event::MULTI_PRESS_TRIPLE );
                break;
            default:
                c.invokeEvent( input::event::MULTI_PRESS_MANY );
                break;
        }
    }
    if ( ( CURRENT_TIME - c.tChange ) > c.tSteadyOn ) {
        c.channelState = &input::channel::digitalSteadyInHighState;
        c.invokeEvent( input::event::STEADY_IN_HIGH );
        c.tChange = CURRENT_TIME;
    }
}
/*============================================================================*/
void input::channel::digitalSteadyInHighState( channel& c )
{
    if ( 0 == c.value ) {
        c.channelState = &input::channel::digitalFallingEdgeState;
        c.invokeEvent( input::event::FALLING_EDGE );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
}
/*============================================================================*/
void input::channel::analogFallingEdgeState( channel& c )
{
    const qOS::clock_t CURRENT_TIME = clock::getTick();

    if ( c.value > c.riseThreshold ) {
        c.channelState = &input::channel::analogRisingEdgeState;
        c.invokeEvent( input::event::RISING_EDGE );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = CURRENT_TIME;
    }
    else if ( c.value > ( c.fallThreshold + c.hysteresis ) ) {
        c.channelState = &input::channel::analogInBandState;
        c.invokeEvent( input::event::IN_BAND );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = CURRENT_TIME;
    }
    else {
        /*Nothing to do here*/
    }

    if ( ( CURRENT_TIME - c.tChange ) > c.tSteadyOff ) {
        c.channelState = &input::channel::analogSteadyInLowState;
        c.invokeEvent( input::event::STEADY_IN_LOW );
        c.tChange = CURRENT_TIME;
    }
}
/*============================================================================*/
void input::channel::analogRisingEdgeState( channel& c )
{
    const qOS::clock_t CURRENT_TIME = clock::getTick();

    if ( c.value < c.fallThreshold ) {
        c.channelState = &input::channel::analogFallingEdgeState;
        c.invokeEvent( input::event::FALLING_EDGE );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = CURRENT_TIME;
    }
    else if ( c.value < ( c.riseThreshold - c.hysteresis ) ) {
        c.channelState = &input::channel::analogInBandState;
        c.invokeEvent( input::event::IN_BAND );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = CURRENT_TIME;
    }
    else {
        /*Nothing to do here*/
    }

    if ( ( CURRENT_TIME - c.tChange ) > c.tSteadyOn ) {
        c.channelState = &input::channel::analogSteadyInHighState;
        c.invokeEvent( input::event::STEADY_IN_HIGH );
        c.tChange = CURRENT_TIME;
    }
}
/*============================================================================*/
void input::channel::analogInBandState( channel& c )
{
    const qOS::clock_t CURRENT_TIME = clock::getTick();

    if ( c.value > c.riseThreshold ) {
        c.channelState = &input::channel::analogRisingEdgeState;
        c.invokeEvent( input::event::RISING_EDGE );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = CURRENT_TIME;
    }
    else if ( c.value < c.fallThreshold ) {
        c.channelState = &input::channel::analogFallingEdgeState;
        c.invokeEvent( input::event::FALLING_EDGE );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = CURRENT_TIME;
    }
    else {
        /*Nothing to do here*/
    }

    if ( ( CURRENT_TIME - c.tChange ) > c.tSteadyBand ) {
        c.channelState = &input::channel::analogSteadyInBandState;
        c.invokeEvent( input::event::STEADY_IN_BAND );
        c.tChange = CURRENT_TIME;
    }
}
/*============================================================================*/
void input::channel::analogSteadyInHighState( channel& c )
{
    if ( c.value < c.fallThreshold ) {
        c.channelState = &input::channel::analogFallingEdgeState;
        c.invokeEvent( input::event::FALLING_EDGE );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
    else if ( c.value < ( c.riseThreshold - c.hysteresis ) ) {
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
void input::channel::analogSteadyInLowState( channel& c )
{
    if ( c.value > c.riseThreshold ) {
        c.channelState = &input::channel::analogRisingEdgeState;
        c.invokeEvent( input::event::RISING_EDGE );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
    else if ( c.value > ( c.fallThreshold + c.hysteresis ) ) {
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
void input::channel::analogSteadyInBandState( channel& c )
{
    if ( c.value > c.riseThreshold ) {
        c.channelState = &input::channel::analogRisingEdgeState;
        c.invokeEvent( input::event::RISING_EDGE );
        c.invokeEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
    else if ( c.value < c.fallThreshold ) {
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
    if ( ( digitalChannels.length() > 0U ) && waitDebounce.freeRun( debounceTime ) ) {
        for ( auto i = digitalChannels.begin(); i.untilEnd() ; i++ ) {
            input::channel& c = *i.get<input::channel*>();
            watchDigital( c );
        }
    }
    if ( analogChannels.length() > 0U ) {
        for ( auto i = analogChannels.begin(); i.untilEnd() ; i++ ) {
            input::channel& c = *i.get<input::channel*>();
            watchAnalog( c );
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
    bool retValue = false;

    if ( input::type::DIGITAL == c.channelType ) {
        auto val = ( nullptr != digitalReader ) ? digitalReader( c.number ) : -1;
        if ( c.negate ) {
            val = !val; // skipcq: CXX-W2065
        }
        c.channelState = ( 0 == val ) ? &input::channel::digitalFallingEdgeState
                                      : &input::channel::digitalRisingEdgeState;
        retValue = digitalChannels.insert( &c );
    }
    else {
        const auto val = ( nullptr != analogReader ) ? analogReader( c.number ) : -1;
        if ( val > c.riseThreshold ) {
            c.channelState = &input::channel::analogRisingEdgeState;
        }
        else if ( val < c.fallThreshold ) {
            c.channelState = &input::channel::analogFallingEdgeState;
        }
        else {
            c.channelState = &input::channel::analogInBandState;
        }
        /* check if channel is shared( same channel number)*/
        for ( auto i = analogChannels.begin(); i.untilEnd() ; i++ ) {
            input::channel& channelInWatcher = *i.get<input::channel*>();

            if ( ( input::type::ANALOG == c.channelType ) && ( c.number == channelInWatcher.number ) ) {
                c.ptrValue = &channelInWatcher.value;
                break;
            }
        }
        retValue = analogChannels.insert( &c );
    }
    c.tChange = clock::getTick();

    return retValue;
}
/*============================================================================*/
bool input::watcher::remove( channel& c ) noexcept
{
    list *channelContainer = c.getContainer();
    bool retValue = channelContainer->remove( &c );
    c.ptrValue = &c.value;

    if ( ( input::type::ANALOG == c.channelType ) && !c.isShared() ) {
        int *newPtrVal = nullptr;
        /*find the next shared channel*/
        for ( auto i = analogChannels.begin(); i.untilEnd() ; i++ ) {
            input::channel& channelInList = *i.get<input::channel*>();

            if ( channelInList.number == c.number ) {
                if ( nullptr == newPtrVal ) { /*first shared channel*/
                    newPtrVal = &channelInList.value;
                    channelInList.ptrValue = &channelInList.value;
                }
                else {
                    channelInList.ptrValue = newPtrVal;
                }
            }
        }
    }

    return retValue;
}
/*============================================================================*/
bool input::watcher::registerEvent( const event e, const eventCallback_t &f, const qOS::duration_t t ) noexcept
{
    bool retValue = true;

    if ( input::event::EXCEPTION == e ) {
        exception = f;
    }
    else {
        for ( auto i = digitalChannels.begin(); i.untilEnd() ; i++ ) {
            input::channel * const c = i.get<input::channel*>();
            retValue &= c->registerEvent( e, f, t );
        }
        for ( auto i = analogChannels.begin(); i.untilEnd() ; i++ ) {
            input::channel * const c = i.get<input::channel*>();
            retValue &= c->registerEvent( e, f, t );
        }
    }

    return retValue;
}
/*============================================================================*/
