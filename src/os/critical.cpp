#include "critical.hpp"

using namespace qOS;
using namespace qOS::critical;

struct _Handler_s {
    int_disabler_t disable;     /*< Point to the user-supplied function used to disable the hardware interrupts. */
    int_restorer_t restore;     /*< Point to the user-supplied function used to restore the hardware interrupts. */
    volatile uint32_t flags;    /*< To save the current interrupt flags before the disable action is performed. */
};
using criticalHandler_t = struct _Handler_s;
static criticalHandler_t c = { nullptr, nullptr , 0uL };

/*============================================================================*/
void critical::enter( void ) noexcept
{
    if ( nullptr != c.disable ) {
        const int_disabler_t xDisabler = c.disable;

        c.flags = xDisabler();
    }
}
/*============================================================================*/
void critical::exit( void ) noexcept
{
    if ( nullptr != c.restore ) {
        const int_restorer_t xRestorer = c.restore;

        xRestorer( c.flags );
    }
}
/*============================================================================*/
bool critical::setInterruptsED( const int_restorer_t rFcn, const int_disabler_t dFcn ) noexcept
{
    bool retValue = false;

    if ( ( rFcn != c.restore ) || ( dFcn != c.disable ) ) {
        c.restore = rFcn;
        c.disable = dFcn;
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
