#include "quarkts++.h"


using namespace qOS;


void idleTask_Callback( event_t e ) {
  if ( e.firstCall() ) {
  
  }

  co::reenter() {
    co::delay( 0.5f );
    co::restart;
  }
}

int main() {
  os.init( nullptr, 0.001f, idleTask_Callback );
  return 0;
}
