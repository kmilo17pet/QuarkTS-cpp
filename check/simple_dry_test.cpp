#include <chrono>
#include <stdio.h>
#include <quarkts++.h>

using namespace std;
using namespace qOS;


task t1, t2, t3, t4;
stateMachine m;
sm::state s1, s2;
co::position pos1;

sm::timeoutStateDefinition_t LedOn_Timeouts[] = {
    { 10000,  sm::TIMEOUT_INDEX( 0 ) | sm::TIMEOUT_SET_ENTRY | sm::TIMEOUT_RST_EXIT },
};

void idleTask_callback( event_t e ) 
{
    if ( e.firstCall() ) {
        trace::log << "idle task" << trace::endl;
    }
    co::reenter() {
        for(;;) {
            co::getPosition( pos1 );
            trace::log << e.self() << trace::endl;
            trace::log << "sec 1"<< trace::endl;
            co::delay( 500 );
            trace::log <<"sec 2"<< trace::endl;
            co::delay( 500 );
            trace::log <<"sec 3"<< trace::endl;
            co::delay( 500 );
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
        trace::log << "idle task" << trace::endl;
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
            trace::log<< trace::cyn <<"s1_callback"<< trace::nrm << trace::endl;
            h.timeoutSet( 0, 5000 );
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

    trace::log << trace::var(tmr) << trace::endl;

    switch ( h.signal() ) {
        case sm::SIGNAL_ENTRY:
            trace::log<< trace::cyn  << "s2_callback"<< trace::nrm << trace::endl;
            tmr( 5000u );
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
    trace::log << e.self() << trace::endl;

    if ( e.firstCall() ) {
        trace::log << trace::grn << "first call "<< e.self().getName() << trace::endl << trace::nrm; 
    }

    if( trigger::byNotificationSimple ==  e.getTrigger() ) {
        trace::log << "notified(SIMPLE)! " << e.self().getName() << trace::endl;
    }

    if( trigger::byNotificationQueued ==  e.getTrigger() ) {
        trace::log << "notified(QUEUED)! " << e.self().getName() << trace::endl;
    }
   
    if ( e.lastIteration() ) {
        os.notify( notifyMode::QUEUED, t1, nullptr );
        os.notify( notifyMode::QUEUED, t1, nullptr );
        os.notify( notifyMode::QUEUED, t2, nullptr );
        os.notify( notifyMode::QUEUED, t1, nullptr );
        trace::log << "last iteration "<< e.self().getName() << trace::endl; 
    }

    int someValue = 457;
    int *ptr = &someValue;
    trace::log << trace::red <<"test trace "<< trace::oct << trace::var(someValue)<< "  " << ptr <<" afdas" << trace::nrm << trace::endl;

}

uint32_t clockProvider( void ) {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

void putCharFcn( void* stp, char c ) {
    (void)stp;
    putchar(c);
}


int main()
{
    trace::setOutputFcn( &putCharFcn );
    t1.setName( "t1");
    t2.setName( "t2");
    t3.setName( "t3");
    os.init( clockProvider, idleTask_callback );

    os.addTask( t1, task_callback, core::LOWEST_PRIORITY, 500u, task::PERIODIC );
    os.addTask( t2, task_callback, core::HIGHEST_PRIORITY, 500u, 10u );
    os.addTask( t3, task_callback, core::MEDIUM_PRIORITY, 2000u, task::PERIODIC );

    sm::timeoutSpec tm_specTimeout;
    m.setup( nullptr, s1 );
    m.installTimeoutSpec( tm_specTimeout );
    m.add( s1, s1_callback );
    m.add( s2, s2_callback );
    os.addStateMachineTask( t4, m, qOS::core::MEDIUM_PRIORITY, 100u );
    
    os.run();
    for(;;) { }

    return 0;
}
