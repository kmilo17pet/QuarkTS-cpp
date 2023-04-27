#include <chrono>
#include <stdio.h>
#include <QuarkTS.h>

using namespace std;

class customTask : public task {
    void activities( void ) override 
    {
        if ( eventData().firstCall() ) {
            trace::log << eventData().thisTask().getName() << trace::endl; 
        }
        trace::log << trace::mag << "im a custom task" << trace::end;
    }
};

task t1, t2, t3, t4;
customTask t5;
stateMachine m;
sm::state s1, s2;
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
    if ( e.firstCall() ) {
        trace::log << "idle task" << trace::end;
    }
    co::reenter() {
        for(;;) {
            co::getPosition( pos1 );
            trace::log << e.thisTask() << trace::end;
            trace::log << "sec 1"<< trace::end;
            co::delay( 0.5_sec );
            trace::log <<"sec 2"<< trace::end;
            co::delay( 0.5_sec );
            trace::log <<"sec 3"<< trace::end;
            co::delay( 0.5_sec );
            co::waitUntil( true == true );
            co::waitUntil( true == true , 500 );
            co::yield;
            co::restart;
            co::setPosition( pos1 );
            if ( co::timeoutExpired() ) {

            }
        }
    }
}

co::handle otherTaskCrHandle;
co::semaphore sem(1);

void otherTask( event_t e )
{
    if ( e.firstCall() ) {
        trace::log << e.thisTask() << trace::end;
    }
    co::reenter( otherTaskCrHandle ) {
        co::restart;
        co::semWait( sem );
        co::semSignal( sem );
    }
}

sm::status s1_callback( sm::handler_t h )
{
    switch ( h.signal() ) {
        case sm::SIGNAL_ENTRY:
            trace::log<< trace::cyn << h.thisMachine() << h.thisState() << "s1_callback"<< trace::end;
            h.timeoutSet( 0, 5_sec );
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

    trace::log << trace::var(tmr) << trace::end;

    switch ( h.signal() ) {
        case sm::SIGNAL_ENTRY:
            trace::log<< trace::cyn  << h.thisMachine() << h.thisState() << "s2_callback"<< trace::end;
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
    trace::log << e.self() << trace::end;

    if ( e.firstCall() ) {
        trace::log << trace::grn << "first call "<< e.thisTask() << trace::end;
    }

    if( trigger::byNotificationSimple ==  e.getTrigger() ) {
        trace::log << "notified(SIMPLE)! " << e.thisTask() << trace::end;
    }

    if( trigger::byNotificationQueued ==  e.getTrigger() ) {
        trace::log << "notified(QUEUED)! " << e.thisTask() << trace::end;
    }
   
    if ( e.lastIteration() ) {
        os.notify( notifyMode::QUEUED, t1, nullptr );
        os.notify( notifyMode::QUEUED, t1, nullptr );
        os.notify( notifyMode::QUEUED, t2, nullptr );
        os.notify( notifyMode::QUEUED, t1, nullptr );
        trace::log << "last iteration "<< e.thisTask() << trace::end;
    }

    int someValue = 457;
    int *ptr = &someValue;
    trace::log << trace::red <<"test trace "<< trace::dec << trace::var(someValue)<< "  " << ptr << trace::end;

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
    trace::setOutputFcn( &putCharFcn );
    trace::log << trace::pre(8) <<trace::var(y) << trace::end; 
    trace::log<< trace::var(x) << trace::mem( sizeof(x) ) << &x << trace::end;
    os.init( sysClock, idleTask_callback );
    
    os.addTask( t1, task_callback, core::LOWEST_PRIORITY, 0.5_sec, task::PERIODIC );
    os.addTask( t2, task_callback, core::HIGHEST_PRIORITY, 0.5_sec, 10u );
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
    os.addStateMachineTask( t4, m, qOS::core::MEDIUM_PRIORITY, 100_ms );

    os.run();
    for(;;) { }

    return 0;
}
