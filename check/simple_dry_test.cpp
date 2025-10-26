#include <chrono>
#include <stdio.h>
#include <QuarkTS.h>

using namespace qOS;

class customTask : public task {
    void activities( event_t e ) override
    {
        if ( e.firstCall() ) {
            logger::out() << e.thisTask();
        }
        logger::out() << logger::mag << "im a custom task";
    }
};

task t1, t2, t3, t4;
customTask t5;
stateMachine m;
sm::state s1, s2;
sm::signalQueue<10> signalQueue;
co::position pos1;

enum : sm::signalIDType {
    SIGNAL_BUTTON_PRESSED = sm::SIGNAL_USER( 1 ),
    SIGNAL_DELAY          = sm::SIGNAL_TIMEOUT( 0 ),
    SIGNAL_BLINK          = sm::SIGNAL_TIMEOUT( 1 ),
};

stateMachine LED_FSM; /*The state-machine handler*/
sm::state State_LEDOff, State_LEDOn, State_LEDBlink;
sm::signalQueue<5> LEDsigqueue;
sm::timeoutSpec tm_spectimeout;

sm::transition LEDOff_transitions[] = {
    { SIGNAL_BUTTON_PRESSED, State_LEDOn }
};

sm::transition LEDOn_transitions[] = {
    { SIGNAL_DELAY,          State_LEDOff   },
    { SIGNAL_BUTTON_PRESSED, State_LEDBlink }
};

sm::transition LEDBlink_transitions[] = {
    { SIGNAL_DELAY,          State_LEDOff  },
    { SIGNAL_BUTTON_PRESSED, State_LEDOff  }
};

sm::timeoutStateDefinition LedOn_Timeouts[] = {
    { 10_sec,  sm::TIMEOUT_USE_SIGNAL( 0 ) | sm::TIMEOUT_SET_ENTRY | sm::TIMEOUT_RST_EXIT },
};

sm::timeoutStateDefinition LEDBlink_timeouts[] = {
    { 10_sec,  sm::TIMEOUT_USE_SIGNAL( 0 ) | sm::TIMEOUT_SET_ENTRY  | sm::TIMEOUT_RST_EXIT  },
    { 0.5_sec, sm::TIMEOUT_USE_SIGNAL( 1 ) | sm::TIMEOUT_SET_ENTRY  | sm::TIMEOUT_RST_EXIT | sm::TIMEOUT_PERIODIC }
};


void idleTask_callback( event_t e );
void otherTask( event_t e );
sm::status s1_callback( sm::handler_t h );
sm::status s2_callback( sm::handler_t h );
void task_callback( event_t e );

void putCharFcn( void* stp, char c );

unsigned long sysClock( void );


void idleTask_callback( event_t e )
{
    unsigned int a;

    if ( e.firstCall() ) {
        logger::out() << "idle task" << a;;
        logger::out() << e.thisTask();
    }
    co::reenter() {
        for(;;) {
            co::getPosition( pos1 );
            logger::out() << e.thisTask();
            logger::out() << "sec 1";
            co::delay( 0.5_sec );
            logger::out() <<"sec 2";
            co::delay( 0.5_sec );
            logger::out() <<"sec 3";
            co::delay( 0.5_sec );
            co::waitUntil( true );
            co::waitUntil( true , 500 );
            co::yield();
            co::restart();
            co::setPosition( pos1 );
            if ( co::timeoutExpired() ) {

            }
            co::perform() {

            }co::until(false);
        }
    }
}

co::handle otherTaskCrHandle;
co::semaphore sem(1);

void otherTask( event_t e )
{
    if ( e.firstCall() ) {
        logger::out() << e.thisTask();
    }
    co::reenter( otherTaskCrHandle ) {
        co::restart();
        co::semWait( sem );
        co::semSignal( sem );
    }
}

sm::status s1_callback( sm::handler_t h )
{
    switch ( h.signal() ) {
        case sm::SIGNAL_ENTRY:
            logger::out() << logger::cyn << h.thisMachine() << h.thisState() << "s1_callback";
            h.thisMachine().timeoutSet( 0, 5_sec );
            break;
        case sm::SIGNAL_TIMEOUT( 0 ):
            h.nextState( s2 );
            break;
        default:
            break;
    }
    return qOS::sm::status::SUCCESS;
}

sm::status s2_callback( sm::handler_t h )
{
    static timer tmr;

    logger::out() << logger::var(tmr);

    switch ( h.signal() ) {
        case sm::SIGNAL_ENTRY:
            logger::out() << logger::cyn  << h.thisMachine() << h.thisState() << "s2_callback";
            tmr( 5_sec );
            break;
        default:
            if ( tmr() ) {
                h.nextState( s1 );
            }
            break;
    }
    return sm::status::SUCCESS;
}

void task_callback( event_t e )
{
    logger::out() << e.self();

    if ( e.firstCall() ) {
        logger::out() << logger::grn << "first call "<< e.thisTask();
    }

    if( trigger::byNotificationSimple ==  e.getTrigger() ) {
        logger::out() << "notified(SIMPLE)! " << e.thisTask();
    }

    if( trigger::byNotificationQueued ==  e.getTrigger() ) {
        logger::out() << "notified(QUEUED)! " << e.thisTask();
    }

    if ( e.lastIteration() ) {
        os.notify( notifyMode::QUEUED, t1, nullptr );
        os.notify( notifyMode::QUEUED, t1, nullptr );
        os.notify( notifyMode::QUEUED, t2, nullptr );
        os.notify( notifyMode::QUEUED, t1, nullptr );
        logger::out() << "last iteration "<< e.thisTask();
    }

    int someValue = 457;
    int *ptr = &someValue;
    logger::out() << logger::red <<"test trace "<< logger::dec << logger::var(someValue)<< "  " << ptr;

}

unsigned long sysClock( void ) {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

void putCharFcn( void* stp, char c ) {
    (void)stp;
    putchar(c);
}


input::digitalValue_t digitalRead( uint8_t c );
input::analogValue_t analogRead( uint8_t c );

input::digitalValue_t digitalRead( uint8_t c ) {
    (void)c;
    return 0;
}

input::analogValue_t analogRead( uint8_t c ) {
    (void)c;
    return 0;
}

input::analogChannel pinA0( 0, 400, 600 );
input::digitalChannel pinD2( 2, true );
input::digitalChannel pinD3( 3, true );
input::digitalChannel pinD4( 4, true );
input::watcher pinWatcher( digitalRead, analogRead, 50_ms );


int main()
{

    uint32_t x = 0xFFAA2211;
    double y = -3.1416;
    logger::setOutputFcn( &putCharFcn );
    logger::out() << logger::pre(8) << logger::var(y);
    logger::out() << logger::var(x) << logger::mem( sizeof(x) ) << &x;
    logger::out(logger::info) << "info message";
    logger::out(logger::error) << "error message";
    logger::out(logger::debug) << "debug message";
    logger::out(logger::verbose) << "verbose message";


    //return 0;

    os.init( sysClock, idleTask_callback );


    LED_FSM.install( LEDsigqueue );
    LED_FSM.install( tm_spectimeout );

    logger::out(logger::debug);
    State_LEDOff.install( LEDOff_transitions );
    State_LEDOn.install( LEDOn_transitions, LedOn_Timeouts );
    State_LEDBlink.install( LEDBlink_transitions, LEDBlink_timeouts );
    logger::out(logger::debug);
    os.add( t1, task_callback, core::LOWEST_PRIORITY, 0.5_sec, task::PERIODIC );
    os.add( t2, task_callback, core::HIGHEST_PRIORITY, 0.5_sec, 10U );
    os.add( t3, task_callback, core::MEDIUM_PRIORITY, 2_sec, task::PERIODIC );
    os.add( t5, nullptr, core::MEDIUM_PRIORITY, 1_sec, task::PERIODIC );

    t1.setName( "t1" );
    t2.setName( "t2" );
    t3.setName( "t3" );
    t4.setName( "t4" );
    t5.setName( "t5" );

    sm::timeoutSpec tm_specTimeout;
    m.setup( nullptr, s1 );
    m.install( tm_specTimeout );
    m.add( s1, s1_callback );
    m.add( s2, s2_callback );
    m.install( signalQueue );
    os.add( t4, m, qOS::core::MEDIUM_PRIORITY, 100_ms );

    os.run();
    for(;;) { }

    return 0;
}
