#include "coroutine.hpp"

using namespace qOS;

/*============================================================================*/
void co::handle::try_restart( void )
{
    try_set( co::BEGINNING );
}
/*============================================================================*/
void co::handle::try_suspend( void )
{
    if ( prev != ctx->label ) {
        prev = ctx->label;
        try_set( co::SUSPENDED );
    }
}
/*============================================================================*/
void co::handle::try_resume( void )
{
    if ( co::SUSPENDED == prev ) {
        try_restart();
    } 
    else {
        try_set( prev );
    }
    prev = co::UNDEFINED;
}
/*============================================================================*/
void co::handle::try_set( co::state p )
{
    ctx->label = p;
}
/*============================================================================*/
void co::semaphore::signal( void )
{
    ++count;
}
/*============================================================================*/
bool co::semaphore::tryLock( void )
{
    bool retValue = false;

    if ( count > static_cast<std::size_t>( 0u ) ) {
        retValue = true; /*break the Wait operation*/
        --count;
    }

    return retValue;
}
/*============================================================================*/
void co::semaphore::set( std::size_t val )
{
    count = val;
}
/*============================================================================*/