#include "include/clock.hpp"

using namespace qOS;

volatile qOS::clock_t clock::sysTick_Epochs = 0u; 
qOS::getTickFcn_t clock::getTick = &internalTick; /* skipcq: CXX-W2009 */

#if ( Q_SETUP_TICK_IN_HERTZ == 1 )
    timingBase_t clock::timingBase = 0uL;
#else
    #if ( Q_SETUP_TIME_CANONICAL == 1 )
        timingBase_t clock::timingBase = 1u;
    #else
        timingBase_t clock::timingBase = 0.001f;
    #endif
#endif

#if ( Q_SETUP_TIME_CANONICAL == 1 )
    const qOS::time_t clock::NONE = 0uL;
    const qOS::time_t clock::IMMEDIATE = 0uL;
#else
    static const qOS::time_t TIME_FIX_VALUE = 0.5f;
    const qOS::time_t clock::NONE = 0.0f;
    const qOS::time_t clock::IMMEDIATE = 0.0f;
#endif
/*============================================================================*/
qOS::clock_t clock::convert2Clock( const qOS::time_t t ) noexcept
{
    #if ( Q_SETUP_TIME_CANONICAL == 1 )
        return static_cast<qOS::clock_t>( t );
    #else
        #if ( Q_SETUP_TICK_IN_HERTZ == 1 )
            return static_cast<qOS::clock_t>( static_cast<qOS::clock_t>( t )*timingBase );
        #else
            qOS::time_t epochs = clock::IMMEDIATE;

            if ( t > clock::IMMEDIATE ) {
                epochs = ( t/timingBase ) + TIME_FIX_VALUE;
            }

            return static_cast<qOS::clock_t>( epochs );
        #endif
    #endif
}
/*============================================================================*/
qOS::time_t clock::convert2Time( const qOS::clock_t t ) noexcept
{
    #if ( Q_SETUP_TIME_CANONICAL == 1 )
        return static_cast<qOS::time_t>( t );
    #else
        #if ( Q_SETUP_TICK_IN_HERTZ == 1 )
            return static_cast<qOS::time_t>( t/timingBase );
        #else
            /*cstat -CERT-FLP36-C*/
            return static_cast<qOS::time_t>( timingBase*static_cast<qOS::time_t>( t ) );
            /*cstat +CERT-FLP36-C*/
        #endif
    #endif
}
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
bool clock::setTimeBase( const timingBase_t tb ) noexcept
{
    bool retValue = false;
    /*cstat -CERT-FLP36-C*/
    if ( tb > static_cast<timingBase_t>( 0 ) ) {
        timingBase = tb;
        retValue = true;
    }
    /*cstat +CERT-FLP36-C*/
    return retValue;
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