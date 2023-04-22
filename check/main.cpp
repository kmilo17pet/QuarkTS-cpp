#include "QuarkTS.h"


using namespace qOS;


void idleTask_Callback( event_t e ) {
  if ( e.firstCall() ) {
  
  }

  co::reenter() {
    co::delay( 500 );
    co::restart;
  }
}

int main() {
  os.init( nullptr, idleTask_Callback );
  return 0;
}
