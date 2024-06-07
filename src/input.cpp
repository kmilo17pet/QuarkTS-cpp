#include "include/input.hpp"

using namespace qOS;

/*============================================================================*/
void input::digitalChannel::updateReading( input::channelReaderFcn_t reader ) noexcept
{
    int sample = reader( number );

    if ( negate ) {
        sample = !sample; // skipcq: CXX-W2065
    }
    value = sample;
}
/*============================================================================*/
void input::analogChannel::updateReading( input::channelReaderFcn_t reader ) noexcept
{
    value = ( isShared() ) ? ptrValue[ 0 ] : reader( number );
}
/*============================================================================*/
void input::digitalChannel::fallingEdgeState( input::digitalChannel& c )
{
    const qOS::clock_t CURRENT_TIME = clock::getTick();

    if ( 0 != c.value ) {
        c.channelState = &input::digitalChannel::risingEdgeState;
        c.dispatchEvent( input::event::RISING_EDGE );
        c.dispatchEvent( input::event::ON_CHANGE );
        if ( ( CURRENT_TIME - c.tChange ) > c.pulsationInterval ) {
            c.pulsationCount = 0U;
        }
        c.tChange = CURRENT_TIME;
    }
    if ( ( CURRENT_TIME - c.tChange ) > c.tSteadyLow ) {
        c.channelState = &input::digitalChannel::steadyInLowState;
        c.dispatchEvent( input::event::STEADY_IN_LOW );
        c.tChange = CURRENT_TIME;
    }
}
/*============================================================================*/
void input::digitalChannel::steadyInLowState( input::digitalChannel& c )
{
    if ( 0 != c.value ) {
        c.channelState = &input::digitalChannel::risingEdgeState;
        c.dispatchEvent( input::event::RISING_EDGE );
        c.dispatchEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
}
/*============================================================================*/
void input::digitalChannel::risingEdgeState( input::digitalChannel& c )
{
    const qOS::clock_t CURRENT_TIME = clock::getTick();

    if ( 0 == c.value ) {
        c.channelState = &input::digitalChannel::fallingEdgeState;
        c.dispatchEvent( input::event::FALLING_EDGE );
        c.dispatchEvent( input::event::ON_CHANGE );
        if ( ( CURRENT_TIME - c.tChange ) <= c.pulsationInterval ) {
            ++c.pulsationCount;
        }
        c.tChange = CURRENT_TIME;

        switch ( c.pulsationCount ) {
            case 0 : case 1: break;
            case 2:
                c.dispatchEvent( input::event::PULSATION_DOUBLE );
                break;
            case 3:
                c.dispatchEvent( input::event::PULSATION_TRIPLE );
                break;
            default:
                c.dispatchEvent( input::event::PULSATION_MULTI );
                break;
        }
    }
    if ( ( CURRENT_TIME - c.tChange ) > c.tSteadyHigh ) {
        c.channelState = &input::digitalChannel::steadyInHighState;
        c.dispatchEvent( input::event::STEADY_IN_HIGH );
        c.tChange = CURRENT_TIME;
    }
}
/*============================================================================*/
void input::digitalChannel::steadyInHighState( input::digitalChannel& c )
{
    if ( 0 == c.value ) {
        c.channelState = &input::digitalChannel::fallingEdgeState;
        c.dispatchEvent( input::event::FALLING_EDGE );
        c.dispatchEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
}
/*============================================================================*/
void input::analogChannel::lowThresholdState( input::analogChannel& c )
{
    const qOS::clock_t CURRENT_TIME = clock::getTick();

    if ( c.value > c.high ) {
        c.channelState = &input::analogChannel::highThresholdState;
        c.dispatchEvent( input::event::HIGH_THRESHOLD );
        c.dispatchEvent( input::event::ON_CHANGE );
        c.tChange = CURRENT_TIME;
    }
    else if ( c.value > ( c.low + c.hysteresis ) ) {
        c.channelState = &input::analogChannel::inBandState;
        c.dispatchEvent( input::event::IN_BAND );
        c.dispatchEvent( input::event::ON_CHANGE );
        c.tChange = CURRENT_TIME;
    }
    else {
        /*Nothing to do here*/
    }

    if ( ( CURRENT_TIME - c.tChange ) > c.tSteadyLow ) {
        c.channelState = &input::analogChannel::steadyInLowState;
        c.dispatchEvent( input::event::STEADY_IN_LOW );
        c.tChange = CURRENT_TIME;
    }
}
/*============================================================================*/
void input::analogChannel::highThresholdState( input::analogChannel& c )
{
    const qOS::clock_t CURRENT_TIME = clock::getTick();

    if ( c.value < c.low ) {
        c.channelState = &input::analogChannel::lowThresholdState;
        c.dispatchEvent( input::event::LOW_THRESHOLD );
        c.dispatchEvent( input::event::ON_CHANGE );
        c.tChange = CURRENT_TIME;
    }
    else if ( c.value < ( c.high - c.hysteresis ) ) {
        c.channelState = &input::analogChannel::inBandState;
        c.dispatchEvent( input::event::IN_BAND );
        c.dispatchEvent( input::event::ON_CHANGE );
        c.tChange = CURRENT_TIME;
    }
    else {
        /*Nothing to do here*/
    }

    if ( ( CURRENT_TIME - c.tChange ) > c.tSteadyHigh ) {
        c.channelState = &input::analogChannel::steadyInHighState;
        c.dispatchEvent( input::event::STEADY_IN_HIGH );
        c.tChange = CURRENT_TIME;
    }
}
/*============================================================================*/
void input::analogChannel::inBandState( input::analogChannel& c )
{
    const qOS::clock_t CURRENT_TIME = clock::getTick();

    if ( c.value > c.high ) {
        c.channelState = &input::analogChannel::highThresholdState;
        c.dispatchEvent( input::event::HIGH_THRESHOLD );
        c.dispatchEvent( input::event::ON_CHANGE );
        c.tChange = CURRENT_TIME;
    }
    else if ( c.value < c.low ) {
        c.channelState = &input::analogChannel::lowThresholdState;
        c.dispatchEvent( input::event::LOW_THRESHOLD );
        c.dispatchEvent( input::event::ON_CHANGE );
        c.tChange = CURRENT_TIME;
    }
    else {
        /*Nothing to do here*/
    }

    if ( ( CURRENT_TIME - c.tChange ) > c.tSteadyBand ) {
        c.channelState = &input::analogChannel::steadyInBandState;
        c.dispatchEvent( input::event::STEADY_IN_BAND );
        c.tChange = CURRENT_TIME;
    }
}
/*============================================================================*/
void input::analogChannel::steadyInHighState( input::analogChannel& c )
{
    if ( c.value < c.low ) {
        c.channelState = &input::analogChannel::lowThresholdState;
        c.dispatchEvent( input::event::LOW_THRESHOLD );
        c.dispatchEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
    else if ( c.value < ( c.high - c.hysteresis ) ) {
        c.channelState = &input::analogChannel::inBandState;
        c.dispatchEvent( input::event::IN_BAND );
        c.dispatchEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
    else {
        /*Nothing to do here*/
    }
}
/*============================================================================*/
void input::analogChannel::steadyInLowState( input::analogChannel& c )
{
    if ( c.value > c.high ) {
        c.channelState = &input::analogChannel::highThresholdState;
        c.dispatchEvent( input::event::HIGH_THRESHOLD );
        c.dispatchEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
    else if ( c.value > ( c.low + c.hysteresis ) ) {
        c.channelState = &input::analogChannel::inBandState;
        c.dispatchEvent( input::event::IN_BAND );
        c.dispatchEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
    else {
        /*Nothing to do here*/
    }
}
/*============================================================================*/
void input::analogChannel::steadyInBandState( input::analogChannel& c )
{
    if ( c.value > c.high ) {
        c.channelState = &input::analogChannel::highThresholdState;
        c.dispatchEvent( input::event::HIGH_THRESHOLD );
        c.dispatchEvent( input::event::ON_CHANGE );
        c.tChange = clock::getTick();
    }
    else if ( c.value < c.low ) {
        c.channelState = &input::analogChannel::lowThresholdState;
        c.dispatchEvent( input::event::LOW_THRESHOLD );
        c.dispatchEvent( input::event::ON_CHANGE );
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
            if ( nullptr != c.callback ) {
                if ( nullptr != digitalReader ) {
                    c.updateReading( digitalReader );
                    c.evaluateState();
                }
                else {
                    c.dispatchEvent( input::event::EXCEPTION );
                }
            }
        }
    }
    if ( analogChannels.length() > 0U ) {
        for ( auto i = analogChannels.begin(); i.untilEnd() ; i++ ) {
            input::channel& c = *i.get<input::channel*>();
            if ( nullptr != c.callback ) {
                if ( ( nullptr != analogReader ) && c.isValidConfig() ) {
                    c.updateReading( analogReader );
                    c.evaluateState();
                }
                else {
                    c.dispatchEvent( input::event::EXCEPTION );
                }
            }
        }
    }

    return true;
}
/*============================================================================*/
void input::digitalChannel::setInitalState( input::channelReaderFcn_t reader ) noexcept
{
    auto val = ( nullptr != reader ) ? reader( number ) : -1;

    if ( negate ) {
        val = !val; // skipcq: CXX-W2065
    }
    channelState = ( 0 == val ) ? &input::digitalChannel::fallingEdgeState
                                : &input::digitalChannel::risingEdgeState;
}
/*============================================================================*/
void input::analogChannel::setInitalState( input::channelReaderFcn_t reader ) noexcept
{
    const auto val = ( nullptr != reader ) ? reader( number ) : -1;

    if ( val > high ) {
        channelState = &input::analogChannel::highThresholdState;
    }
    else if ( val < low ) {
        channelState = &input::analogChannel::lowThresholdState;
    }
    else {
        channelState = &input::analogChannel::inBandState;
    }
}
/*============================================================================*/
bool input::watcher::add( input::channel& c ) noexcept
{
    bool retValue;

    if ( input::type::DIGITAL == c.getType() ) {
        c.setInitalState( digitalReader );
        retValue = digitalChannels.insert( &c );
    }
    else {
        c.setInitalState( analogReader );
        /* check if channel is shared( same channel number)*/
        for ( auto i = analogChannels.begin(); i.untilEnd() ; i++ ) {
            input::channel& channelInWatcher = *i.get<input::channel*>();

            if ( c.number == channelInWatcher.number  ) {
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
bool input::watcher::remove( input::channel& c ) noexcept
{
    list *channelContainer = c.getContainer();
    const bool retValue = channelContainer->remove( &c );
    c.ptrValue = &c.value;

    if ( ( input::type::ANALOG == c.getType() ) && !c.isShared() ) {
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
bool input::digitalChannel::setTime( const input::event e, const qOS::duration_t t ) noexcept
{
    bool retValue = true;

    if ( t > 0U ) {
        switch( e ) {
            case input::event::PULSATION_DOUBLE: // skipcq: CXX-C1001
            case input::event::PULSATION_TRIPLE: // skipcq: CXX-C1001
            case input::event::PULSATION_MULTI: // skipcq: CXX-C1001
                pulsationInterval = static_cast<qOS::clock_t>( t );
                break;
            case input::event::STEADY_IN_HIGH:
                tSteadyHigh = static_cast<qOS::clock_t>( t );
                break;
            case input::event::STEADY_IN_LOW:
                tSteadyHigh = static_cast<qOS::clock_t>( t );
                break;
            default:
                retValue = false;
                break;
        }
    }
    else {
        retValue = false;
    }

    return retValue;
}
/*============================================================================*/
bool input::analogChannel::setTime( const input::event e, const qOS::duration_t t ) noexcept
{
    bool retValue = true;

    if ( t > 0U ) {
        switch( e ) {
            case input::event::STEADY_IN_BAND:
                tSteadyBand = static_cast<qOS::clock_t>( t );
                break;
            case input::event::STEADY_IN_HIGH:
                tSteadyHigh = static_cast<qOS::clock_t>( t );
                break;
            case input::event::STEADY_IN_LOW:
                tSteadyHigh = static_cast<qOS::clock_t>( t );
                break;
            default:
                retValue = false;
                break;
        }
    }
    else {
        retValue = false;
    }

    return retValue;
}
/*============================================================================*/
bool input::digitalChannel::setParameter( const input::event e, const int p ) noexcept
{
    (void)e;
    (void)p;
    return false;
}
/*============================================================================*/
bool input::analogChannel::setParameter( const input::event e, const int p ) noexcept
{
    bool retValue = true;

    switch( e ) {
        case input::event::HIGH_THRESHOLD:
            high = p;
            break;
        case input::event::LOW_THRESHOLD:
            low = p;
            break;
        case input::event::IN_BAND:
            hysteresis = p;
            break;
        default:
            retValue = false;
            break;
    }

    return retValue;
}
/*============================================================================*/