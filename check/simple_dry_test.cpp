#include <iostream>
#include <chrono>
#include "quarkts++.h"

using namespace std;
using namespace qOS;


task t1, t2, t3, t4;
stateMachine m;
sm::state s1, s2;
co::position pos1;

sm::timeoutStateDefinition_t LedOn_Timeouts[] = {
    { 10.0f,  sm::TIMEOUT_INDEX( 0 ) | sm::TIMEOUT_SET_ENTRY | sm::TIMEOUT_RST_EXIT },
};

void idleTask_callback( event_t e ) 
{
    if ( e.firstCall() ) {
        cout << "idle task" << endl;
    }
    co::reenter() {
        for(;;) {
            co::getPosition( pos1 );
            cout<<"hello 1 "<< endl;
            co::delay( 0.5f );
            cout<<"hello 2 "<< endl;
            co::delay( 0.5f );
            cout<<"hello 3 "<< endl;
            co::delay( 0.5f );
            co::waitUntil( true == true );
            co::waitUntil( true == true , 0.5f );
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
        cout << "idle task" << endl;
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
            cout<<"s1_callback"<<endl;
            h.timeoutSet( 0, 5.0f );
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
    switch ( h.signal() ) {
        case sm::SIGNAL_ENTRY:
            cout<<"s2_callback"<<endl;
            tmr( 5.0f );
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
    if ( e.firstCall() ) {
        cout << "first call "<< e.self().getName() << endl; 
    }

    if( trigger::byNotificationSimple ==  e.getTrigger() ) {
        cout << "notified(SIMPLE)! " << e.self().getName() << endl;
    }

    if( trigger::byNotificationQueued ==  e.getTrigger() ) {
        
        cout << "notified(QUEUED)! " << e.self().getName() << endl;
    }

    cout << "im task "<< e.self().getName() << endl;
    
    if ( e.lastIteration() ) {
        os.notify( notifyMode::QUEUED, t1, nullptr );
        os.notify( notifyMode::QUEUED, t1, nullptr );
        os.notify( notifyMode::QUEUED, t2, nullptr );
        os.notify( notifyMode::QUEUED, t1, nullptr );
        cout << "last iteration "<< e.self().getName() << endl; 
    }
}

uint32_t clockProvider( void ) {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}


int main()
{
    t1.setName( "t1");
    t2.setName( "t2");
    t3.setName( "t3");
    os.init( clockProvider, 0.001f, idleTask_callback );

    os.addTask( t1, task_callback, core::LOWEST_PRIORITY, 0.5f, task::PERIODIC );
    os.addTask( t2, task_callback, core::HIGHEST_PRIORITY, 0.5f, 10 );
    os.addTask( t3, task_callback, core::MEDIUM_PRIORITY, 2.0f, task::PERIODIC );

    sm::timeoutSpec tm_specTimeout;
    m.setup( nullptr, s1 );
    m.installTimeoutSpec( tm_specTimeout );
    m.add( s1, s1_callback );
    m.add( s2, s2_callback );
    os.addStateMachineTask( t4, m, qOS::core::MEDIUM_PRIORITY, 0.1f );
    
    os.run();
    for(;;) { }

    return 0;
}
