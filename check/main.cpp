#include "QuarkTS.h"


using namespace qOS;


void idleTask_Callback( event_t e ) {
  e.self().setTime( 5_sec );
  if ( e.firstCall() ) {
    logger::out() << "first call";
  }
  int x = 0;
  co::reenter() {
    co::delay( 500 );
    co::restart();
    co::waitUntil( false );
    co::perform(2.5_sec) {
      logger::out() << "hello world";
    } co::until( false );
  }
}

int main() {
  (void)os.init( nullptr, idleTask_Callback );
  return 0;
}
