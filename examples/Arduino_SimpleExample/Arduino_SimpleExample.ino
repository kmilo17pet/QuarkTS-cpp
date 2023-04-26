#include <QuarkTS.h>

using namespace qOS;
task demoTask1, blinkTask;

void blinkTask_callback( event_t e ) {
  if ( e.firstCall() ) {
     trace::log << e.thisTask() << " Launched!" << trace::endl;
  }
  co::reenter() {
    for(;;) {
      digitalWrite( LED_BUILTIN, HIGH );
      trace::log << e.thisTask() << trace::endl;
      co::delay( 500_ms );
      digitalWrite( LED_BUILTIN, LOW );
      trace::log << e.thisTask() << trace::endl;
      co::delay( 500_ms );
    }
  }
}

void demoTask1_Callback( event_t e ) {
  if ( e.firstCall() ) {
     trace::log << e.thisTask() << " Launched!" << trace::endl;
  }
  trace::log << e.thisTask() << trace::endl;
}

void idleTask_callback( event_t e ) {
  if ( e.firstCall() ) {
     trace::log << e.thisTask() << " Launched!" << trace::endl;
  }
  co::reenter() {
    for(;;) {
      co::delay( 500_ms );
      trace::log << e.thisTask() << trace::endl;
    }
  }
}

/* hardware-OS wrappers*/
void tracePutcWrapper( void *arg, const char c ) {
  Serial.write( c );
  (void)arg;
}
void uint32_t disableINTWrapper( void ) {
  noInterrupts();
}
void restoreINTWrapper( uint32_t s ) {
  interrupts();
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  trace::setOutputFcn( tracePutcWrapper );
  critical::setInterruptsED( resetINTWrapper, disableINTWrapper );
  os.init( millis, idleTask_callback );
  demoTask1.setName("demoTask1");
  blinkTask.setName("blinkTask");
  os.addTask( demoTask1, demoTask1_Callback, core::MEDIUM_PRIORITY, 250_ms, task::PERIODIC );
  os.addTask( blinkTask, blinkTask_callback, core::LOWEST_PRIORITY, 10_ms, task::PERIODIC );
}

void loop() {
  os.run();
}
