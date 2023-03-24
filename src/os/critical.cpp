#include "critical.hpp"

using namespace qOS;
using namespace qOS::critical;

typedef struct _Handler_s {
    int_disabler_t disable;     /*< Point to the user-supplied function used to disable the hardware interrupts. */
    int_restorer_t restore;     /*< Point to the user-supplied function used to restore the hardware interrupts. */
    volatile std::uint32_t flags;    /*< To save the current interrupt flags before the disable action is performed. */
} criticalHandler_t;

static criticalHandler_t c = { nullptr, nullptr , 0uL };

/*============================================================================*/
void critical::enter( void )
{
    if ( nullptr != c.disable ) {
        int_disabler_t xDisabler = c.disable;

        c.flags = xDisabler();
    }
}
/*============================================================================*/
void critical::exit( void )
{
    if ( nullptr != c.restore ) {
        int_restorer_t xRestorer = c.restore;

        xRestorer( c.flags );
    }
}
/*============================================================================*/
bool critical::setInterruptsED( const int_restorer_t rFcn, const int_disabler_t dFcn )
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
