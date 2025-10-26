#include "include/critical.hpp"

using namespace qOS;

/*! @cond  */
int_disabler_t critical::lock::disable = nullptr; /*< Point to the user-supplied function used to disable the hardware interrupts. */
int_restorer_t critical::lock::restore = nullptr; /*< Point to the user-supplied function used to restore the hardware interrupts. */
volatile uint32_t critical::lock::flags = 0UL;    /*< To save the current interrupt flags before the disable action is performed. */
volatile int critical::lock::nestingLevel = 0;
/*! @endcond  */

/*============================================================================*/
void critical::lock::enter( void ) noexcept
{
    if ( ( 0 == nestingLevel ) && ( nullptr != disable ) ) {
        const int_disabler_t xDisabler = disable;
        flags = xDisabler();
    }
    ++nestingLevel;
}
/*============================================================================*/
void critical::lock::exit( void ) noexcept
{
    if ( nestingLevel > 0 ) {
        --nestingLevel;
        if ( ( 0 == nestingLevel ) && ( nullptr != restore ) ) {
            const int_restorer_t xRestorer = restore;
            xRestorer( flags );
        }
    }
}
/*============================================================================*/
critical::lock::lock() noexcept
{
    enter();
}
/*============================================================================*/
critical::lock::~lock() noexcept
{
    exit();
}
/*============================================================================*/
critical::lock::operator bool() noexcept
{
    const bool ret = entered;
    entered = false;
    return ret;
}
/*============================================================================*/
bool critical::setInterruptsED( const int_restorer_t rFcn, const int_disabler_t dFcn ) noexcept
{
    bool retValue = false;

    if ( ( rFcn != critical::lock::restore ) || ( dFcn != critical::lock::disable ) ) {
        critical::lock::restore = rFcn;
        critical::lock::disable = dFcn;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
