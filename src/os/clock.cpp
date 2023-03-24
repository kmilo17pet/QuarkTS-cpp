#include "clock.hpp"

using namespace qOS;

static const qOS::time_t TIME_FIX_VALUE = 0.5f;
static qOS::clock_t internalTick( void );
static volatile qOS::clock_t sysTick_Epochs  = 0u;
static timingBase_t timingBase;

qOS::getTickFcn_t clock::getTick = &internalTick;

#if ( Q_SETUP_TIME_CANONICAL == 1 )
    const qOS::time_t clock::NONE = 0uL;
    const qOS::time_t clock::IMMEDIATE = 0uL;
    const qOS::time_t clock::MINUTE = 60000uL;
    const qOS::time_t clock::HOUR = 3600000uL;
    const qOS::time_t clock::DAY = 86400000uL;
    const qOS::time_t clock::WEEK = 604800000uL;
#else
    const qOS::time_t clock::NONE = 0.0f;
    const qOS::time_t clock::IMMEDIATE = 0.0f;
    const qOS::time_t clock::MINUTE = 60.0f;
    const qOS::time_t clock::HOUR = 3600.0f;
    const qOS::time_t clock::DAY = 86400.0f;
    const qOS::time_t clock::WEEK = 604800.0f;
#endif
/*============================================================================*/
qOS::clock_t clock::convert2Clock( const qOS::time_t t )
{
    #if ( Q_SETUP_TIME_CANONICAL == 1 )
        return (qOS::clock_t)t;
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
qOS::time_t clock::convert2Time( const qOS::clock_t t )
{
    #if ( Q_SETUP_TIME_CANONICAL == 1 )
        return static_cast<qOS::time_t>( t );
    #else
        #if ( Q_SETUP_TICK_IN_HERTZ == 1 )
            return static_cast<qOS::time_t>( t/timingBase );
        #else
            return static_cast<qOS::time_t>( timingBase*static_cast<qOS::time_t>( t ) );
        #endif
    #endif
}
/*============================================================================*/
void clock::sysTick( void )
{
    ++sysTick_Epochs;
}
/*============================================================================*/
bool clock::timeDeadLineCheck( const qOS::clock_t ti, const qOS::clock_t td )
{
    return ( clock::getTick() - ti >= td );
}
/*============================================================================*/
bool clock::setTimeBase( const timingBase_t tb )
{
    bool retValue = false;

    if ( tb > static_cast<timingBase_t>( 0 ) ) {
        timingBase = tb;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
static qOS::clock_t internalTick( void )
{
    return sysTick_Epochs;
}
/*============================================================================*/
bool clock::setTickProvider( const getTickFcn_t provider )
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