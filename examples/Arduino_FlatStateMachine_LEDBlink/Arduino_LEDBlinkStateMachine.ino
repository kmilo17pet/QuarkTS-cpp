#include <QuarkTS.h>

using namespace qOS;

const int BUTTON_PIN = 2;
/*define the FSM application event-signals*/
enum : sm::signalIDType {
    SIGNAL_BUTTON_PRESSED = sm::SIGNAL_USER( 1 ),
    SIGNAL_DELAY          = sm::SIGNAL_TIMEOUT( 0 ),
    SIGNAL_BLINK          = sm::SIGNAL_TIMEOUT( 1 ),
};

/*
This example uses a push button (tact switch) attached to digital pin 2 and GND,
using an internal pull-up resistor so pin 2 is HIGH when the button is not pressed.
*/

task LED_Task; /*The task node*/
stateMachine LED_FSM; /*The state-machine handler*/
sm::state State_LEDOff, State_LEDOn, State_LEDBlink;
sm::signalQueue<5> LEDsigqueue;
sm::timeoutSpec tm_spectimeout;
 
/*create the transition tables for every state*/
sm::transition LEDOff_transitions[] = {
    { SIGNAL_BUTTON_PRESSED, &State_LEDOn }
};
 
sm::transition LEDOn_transitions[] = {
    { SIGNAL_DELAY,          &State_LEDOff   },
    { SIGNAL_BUTTON_PRESSED, &State_LEDBlink }
};
 
sm::transition LEDBlink_transitions[] = {
    { SIGNAL_DELAY,          &State_LEDOff  },
    { SIGNAL_BUTTON_PRESSED, &State_LEDOff  }
};
 
/*define the timeout specifications */
sm::timeoutStateDefinition LedOn_Timeouts[] = {
    { 10_sec,  sm::TIMEOUT_INDEX( 0 ) | sm::TIMEOUT_SET_ENTRY | sm::TIMEOUT_RST_EXIT },
};
 
sm::timeoutStateDefinition LEDBlink_timeouts[] = {
    { 10_sec,  sm::TIMEOUT_INDEX( 0 ) | sm::TIMEOUT_SET_ENTRY  | sm::TIMEOUT_RST_EXIT  },
    { 0.5_sec, sm::TIMEOUT_INDEX( 1 ) | sm::TIMEOUT_SET_ENTRY  | sm::TIMEOUT_RST_EXIT | sm::TIMEOUT_PERIODIC }
};

sm::status State_LEDOff_Callback( sm::handler_t h ) {
    switch ( h.signal() ) {
        case sm::signalID::SIGNAL_ENTRY:
            logger::out() << "LED Off state" << logger::end;
            digitalWrite( LED_BUILTIN, LOW );
            break;
        default:
            break;
    }
    return sm::status::SUCCESS;
}

sm::status State_LEDOn_Callback( sm::handler_t h ) {
    switch ( h.signal() ) {
        case sm::signalID::SIGNAL_ENTRY:
            logger::out() << "LED On state" << logger::end;
            digitalWrite( LED_BUILTIN, HIGH );
            break;
        default:
            break;
    }
    return sm::status::SUCCESS;
}

sm::status State_LEDBlink_Callback( sm::handler_t h ) {
    switch ( h.signal() ) {
        case sm::signalID::SIGNAL_ENTRY:
            logger::out() << "LED blink state" << logger::end;
            break;
        case SIGNAL_BLINK:
            digitalWrite( LED_BUILTIN, !digitalRead(LED_BUILTIN) );
            break;
        default:
            break;
    }
    return sm::status::SUCCESS;
}

/* hardware-OS wrappers*/
void tracePutcWrapper( void *arg, const char c ) {
  Serial.write( c );
  (void)arg;
}

void buttonChangeISR() {
    LED_FSM.sendSignal( SIGNAL_BUTTON_PRESSED );
}

void setup() {
  pinMode( LED_BUILTIN, OUTPUT );
  pinMode( BUTTON_PIN, INPUT_PULLUP );
  attachInterrupt( digitalPinToInterrupt( BUTTON_PIN ), buttonChangeISR, FALLING ); // trigger when button pressed, but not when released.
  Serial.begin(115200);

  logger::setOutputFcn( tracePutcWrapper );
  os.init( millis );
  LED_Task.setName("LEDFSM_Task1");
  LED_FSM.setup( nullptr, State_LEDOff );
  LED_FSM.add( State_LEDOff, State_LEDOff_Callback );
  LED_FSM.add( State_LEDOn, State_LEDOn_Callback );
  LED_FSM.add( State_LEDBlink, State_LEDBlink_Callback );
  
  LED_FSM.installSignalQueue( LEDsigqueue );
  LED_FSM.installTimeoutSpec( tm_spectimeout );
  State_LEDOn.setTimeouts( LedOn_Timeouts );
  State_LEDBlink.setTimeouts( LEDBlink_timeouts );
  
  State_LEDOff.setTransitions( LEDOff_transitions );
  State_LEDOn.setTransitions( LEDOn_transitions );
  State_LEDBlink.setTransitions( LEDBlink_transitions );
  
  os.addStateMachineTask( LED_Task, LED_FSM, core::MEDIUM_PRIORITY, 100_ms );
}

void loop() {
  os.run();
}
