#include <QuarkTS.h>

using namespace qOS;
task demoTask1, blinkTask;

void blinkTask_callback( event_t e ) {
  if ( e.firstCall() ) {
     trace::log << e.self() << " Launched!" << trace::endl;
  }
  co::reenter() {
    for(;;) {
      digitalWrite( LED_BUILTIN, HIGH );
      trace::log << e.self() << trace::endl;
      co::delay( 500 );
      digitalWrite( LED_BUILTIN, LOW );
      trace::log << e.self() << trace::endl;
      co::delay( 500 );
    }
  }
}

void demoTask1_Callback( event_t e ) {
  if ( e.firstCall() ) {
     trace::log << e.self() << " Launched!" << trace::endl;
  }
  trace::log << e.self() << trace::endl;
}

void idleTask_callback( event_t e ) {
  if ( e.firstCall() ) {
     trace::log << e.self() << " Launched!" << trace::endl;
  }
  co::reenter() {
    for(;;) {
      co::delay( 500 );
      trace::log << e.self() << trace::endl;
    }
  }
}

void tracePutcWrapper( void *arg, const char c ) {
  Serial.write( c );
  (void)arg;
}

// the setup function runs once when you press reset or power the board
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  trace::setOutputFcn( tracePutcWrapper );
  os.init( millis, idleTask_callback );
  demoTask1.setName("demoTask1");
  blinkTask.setName("blinkTask");
  os.addTask( demoTask1, demoTask1_Callback, core::MEDIUM_PRIORITY, 250, task::PERIODIC );
  os.addTask( blinkTask, blinkTask_callback, core::LOWEST_PRIORITY, 10, task::PERIODIC );
}

// the loop function runs over and over again forever
void loop() {
  os.run();
}
