#include <QuarkTS.h>

using namespace qOS;
task demoTask1, blinkTask;

void blinkTask_callback( event_t e ) {
  if ( e.firstCall() ) {
     logger::out() << e.thisTask() << " Launched!" << logger::endl;
  }
  co::reenter() {
    for(;;) {
      digitalWrite( LED_BUILTIN, HIGH );
      logger::out() << e.thisTask() << logger::endl;
      co::delay( 500_ms );
      digitalWrite( LED_BUILTIN, LOW );
      logger::out() << e.thisTask() << logger::endl;
      co::delay( 500_ms );
    }
  }
}

void demoTask1_Callback( event_t e ) {
  if ( e.firstCall() ) {
     logger::out() << e.thisTask() << " Launched!" << logger::endl;
  }
  logger::out() << e.thisTask() << logger::endl;
}

void idleTask_callback( event_t e ) {
  if ( e.firstCall() ) {
     logger::out() << e.thisTask() << " Launched!" << logger::endl;
  }
  co::reenter() {
    for(;;) {
      co::delay( 500_ms );
      logger::out() << e.thisTask() << logger::endl;
    }
  }
}

/* hardware-OS wrappers*/
void tracePutcWrapper( void *arg, const char c ) {
  Serial.write( c );
  (void)arg;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  logger::setOutputFcn( tracePutcWrapper );
  os.init( millis, idleTask_callback );
  os.addTask( demoTask1, demoTask1_Callback, core::MEDIUM_PRIORITY, 250_ms, task::PERIODIC );
  os.addTask( blinkTask, blinkTask_callback, core::LOWEST_PRIORITY, 10_ms, task::PERIODIC );
  demoTask1.setName("demoTask1");
  blinkTask.setName("blinkTask");
}

void loop() {
  os.run();
}
