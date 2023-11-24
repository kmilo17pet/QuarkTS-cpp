#include <chrono>
#include <stdio.h>
#include <QuarkTS.h>

using namespace std;

class customTask : public task {
    void activities( void ) override 
    {
        if ( eventData().firstCall() ) {
            logger::out() << eventData().thisTask().getName() << logger::endl; 
        }
        logger::out() << logger::mag << "im a custom task" << logger::end;
    }
};

task t1, t2, t3, t4;
customTask t5;
stateMachine m;
sm::state s1, s2;
sm::signalQueue<10> signalQueue;
co::position pos1;

sm::timeoutStateDefinition_t LedOn_Timeouts[] = {
    { 10000,  sm::TIMEOUT_INDEX( 0 ) | sm::TIMEOUT_SET_ENTRY | sm::TIMEOUT_RST_EXIT },
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
        logger::out() << "idle task" << a << logger::end;
    }
    co::reenter() {
        for(;;) {
            co::getPosition( pos1 );
            logger::out() << e.thisTask() << logger::end;
            logger::out() << "sec 1"<< logger::end;
            co::delay( 0.5_sec );
            logger::out() <<"sec 2"<< logger::end;
            co::delay( 0.5_sec );
            logger::out() <<"sec 3"<< logger::end;
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
        logger::out() << e.thisTask() << logger::end;
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
            logger::out() << logger::cyn << h.thisMachine() << h.thisState() << "s1_callback"<< logger::end;
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

    logger::out() << logger::var(tmr) << logger::end;

    switch ( h.signal() ) {
        case sm::SIGNAL_ENTRY:
            logger::out() << logger::cyn  << h.thisMachine() << h.thisState() << "s2_callback"<< logger::end;
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
    logger::out() << e.self() << logger::end;

    if ( e.firstCall() ) {
        logger::out() << logger::grn << "first call "<< e.thisTask() << logger::end;
    }

    if( trigger::byNotificationSimple ==  e.getTrigger() ) {
        logger::out() << "notified(SIMPLE)! " << e.thisTask() << logger::end;
    }

    if( trigger::byNotificationQueued ==  e.getTrigger() ) {
        logger::out() << "notified(QUEUED)! " << e.thisTask() << logger::end;
    }
   
    if ( e.lastIteration() ) {
        os.notify( notifyMode::QUEUED, t1, nullptr );
        os.notify( notifyMode::QUEUED, t1, nullptr );
        os.notify( notifyMode::QUEUED, t2, nullptr );
        os.notify( notifyMode::QUEUED, t1, nullptr );
        logger::out() << "last iteration "<< e.thisTask() << logger::end;
    }

    int someValue = 457;
    int *ptr = &someValue;
    logger::out() << logger::red <<"test trace "<< logger::dec << logger::var(someValue)<< "  " << ptr << logger::end;

}

unsigned long sysClock( void ) {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

void putCharFcn( void* stp, char c ) {
    (void)stp;
    putchar(c);
}

int main()
{
    uint32_t x = 0xFFAA2211;
    double y = -3.1416;
    logger::setOutputFcn( &putCharFcn );
    logger::out() << logger::pre(8) << logger::var(y) << logger::end; 
    logger::out() << logger::var(x) << logger::mem( sizeof(x) ) << &x << logger::end;
    logger::out(logger::info) << "info message"<< logger::end;
    logger::out(logger::error) << "error message"<< logger::end;
    logger::out(logger::debug) << "debug message"<< logger::end;
    logger::out(logger::verbose) << "verbose message"<< logger::end;

    os.init( sysClock, idleTask_callback );
    
    os.addTask( t1, task_callback, core::LOWEST_PRIORITY, 0.5_sec, task::PERIODIC );
    os.addTask( t2, task_callback, core::HIGHEST_PRIORITY, 0.5_sec, 10U );
    os.addTask( t3, task_callback, core::MEDIUM_PRIORITY, 2_sec, task::PERIODIC );
    os.addTask( t5, nullptr, core::MEDIUM_PRIORITY, 1_sec, task::PERIODIC );

    t1.setName( "t1" );
    t2.setName( "t2" );
    t3.setName( "t3" );
    t4.setName( "t4" );
    t5.setName( "t5" );

    sm::timeoutSpec tm_specTimeout;
    m.setup( nullptr, s1 );
    m.installTimeoutSpec( tm_specTimeout );
    m.add( s1, s1_callback );
    m.add( s2, s2_callback );
    m.installSignalQueue( signalQueue );
    os.addStateMachineTask( t4, m, qOS::core::MEDIUM_PRIORITY, 100_ms );

    os.run();
    for(;;) { }

    return 0;
}
