#include <QuarkTS.h>

using namespace qOS;

/*define the FSM application event-signals*/
#define SIGNAL_BUTTON_PRESSED   ( sm::SIGNAL_USER( 1 ) )
#define SIGNAL_DELAY            ( sm::SIGNAL_TIMEOUT( 0 ) )
#define SIGNAL_BLINK            ( sm::SIGNAL_TIMEOUT( 1 ) )
 
task LED_Task; /*The task node*/
stateMachine LED_FSM; /*The state-machine handler*/
sm::state State_LEDOff, State_LEDOn, State_LEDBlink;
sm::signalQueue<5> LEDsigqueue;
sm::timeoutSpec tm_spectimeout;
 
/*create the transition tables for every state*/
sm::transition_t LEDOff_transitions[] = {
    { SIGNAL_BUTTON_PRESSED, nullptr, &State_LEDOn , sm::NO_HISTORY, nullptr }
};
 
sm::transition_t LEDOn_transitions[] = {
    { SIGNAL_DELAY,          nullptr, &State_LEDOff   , sm::NO_HISTORY, nullptr},
    { SIGNAL_BUTTON_PRESSED, nullptr, &State_LEDBlink , sm::NO_HISTORY, nullptr}
};
 
sm::transition_t LEDBlink_transitions[] = {
    { SIGNAL_DELAY,          nullptr, &State_LEDOff   , sm::NO_HISTORY, nullptr},
    { SIGNAL_BUTTON_PRESSED, nullptr, &State_LEDOff   , sm::NO_HISTORY, nullptr}
};
 
/*define the timeout specifications */
sm::timeoutStateDefinition_t LedOn_Timeouts[] = {
    { 10_sec,  sm::TIMEOUT_INDEX( 0 ) | sm::TIMEOUT_SET_ENTRY | sm::TIMEOUT_RST_EXIT },
};
 
sm::timeoutStateDefinition_t LEDBlink_timeouts[] = {
    { 10_sec,  sm::TIMEOUT_INDEX( 0 ) | sm::TIMEOUT_SET_ENTRY  | sm::TIMEOUT_RST_EXIT  },
    { 0.5_sec, sm::TIMEOUT_INDEX( 1 ) | sm::TIMEOUT_SET_ENTRY  | sm::TIMEOUT_RST_EXIT | sm::TIMEOUT_PERIODIC }
};

sm::status State_LEDOff_Callback( sm::handler_t h ) {
    switch ( h.signal() ) {
        case sm::signalID::SIGNAL_ENTRY:
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
            digitalWrite( LED_BUILTIN, HIGH );
            break;
        default:
            break;
    }
    return sm::status::SUCCESS;
}

sm::status State_LEDBlink_Callback( sm::handler_t h ) {
    switch ( h.signal() ) {
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

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
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
  State_LEDOn.setTimeouts( LedOn_Timeouts, arraySize(LedOn_Timeouts) );
  State_LEDBlink.setTimeouts( LEDBlink_timeouts, arraySize(LEDBlink_timeouts) );
  
  State_LEDOff.setTransitions( LEDOff_transitions, arraySize(LEDOff_transitions) );
  State_LEDOn.setTransitions( LEDOn_transitions, arraySize(LEDOn_transitions) );
  State_LEDBlink.setTransitions( LEDBlink_transitions, arraySize(LEDBlink_transitions) );
  
  os.addStateMachineTask( LED_Task, LED_FSM, core::MEDIUM_PRIORITY, 100_ms );
}

void loop() {
  os.run();
}
