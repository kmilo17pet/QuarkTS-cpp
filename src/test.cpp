#include <iostream>
#include <chrono>
#include "kernel.hpp"

using namespace std;

task t1, t2, t3, t4;
qOS::stateMachine m;
qOS::fsm::state s1, s2;

void idleTask_callback( event_t e ) 
{
    if ( e.firstCall() ) {
        cout << "idle task" << endl;
    }
}

qOS::fsm::status s1_callback( qOS::fsm::handler_t h )
{
    switch ( h.signal() ) {
        case qOS::fsm::SIGNAL_ENTRY:
            cout<<"s1_callback"<<endl;
            h.timeoutSet( 0, 5.0f );
            break;
        case qOS::fsm::SIGNAL_TIMEOUT( 0 ):
            h.nextState( &s2 );
            break;
        default:
            break;
    }
    return qOS::fsm::status::SUCCESS;
}

qOS::fsm::status s2_callback( qOS::fsm::handler_t h )
{
    static qOS::timer tmr;
    switch ( h.signal() ) {
        case qOS::fsm::SIGNAL_ENTRY:
            cout<<"s2_callback"<<endl;
            tmr( 5.0f );
        default:
            if ( tmr() ) {
                h.nextState( &s1 );
            }
            break;
    }
    return qOS::fsm::status::SUCCESS;
}

void task_callback( event_t e )
{
    if ( e.firstCall() ) {
        cout << "first call "<< qOS::os.getTaskRunning().getName() << endl; 
    }

    if( trigger::byNotificationSimple ==  e.getTrigger() ) {
        cout << "notified(SIMPLE)! " << qOS::os.getTaskRunning().getName() << endl;
    }

    if( trigger::byNotificationQueued ==  e.getTrigger() ) {
        
        cout << "notified(QUEUED)! " << qOS::os.getTaskRunning().getName() << endl;
    }

    cout << "im task "<< qOS::os.getTaskRunning().getName() << endl;
    
    if ( e.lastIteration() ) {
        qOS::os.notify( qOS::QUEUED, t1, nullptr );
        qOS::os.notify( qOS::QUEUED, t1, nullptr );
        qOS::os.notify( qOS::QUEUED, t2, nullptr );
        qOS::os.notify( qOS::QUEUED, t1, nullptr );
        cout << "last iteration "<< qOS::os.getTaskRunning().getName() << endl; 
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
    qOS::os.init( clockProvider, 0.001f, idleTask_callback );
    qOS::os.addTask( t1, task_callback, qOS::core::LOWEST_PRIORITY, 0.5f, task::PERIODIC, qOS::ENABLED, nullptr );
    qOS::os.addTask( t2, task_callback, qOS::core::HIGHEST_PRIORITY, 0.5f, 10, qOS::ENABLED, nullptr );
    qOS::os.addTask( t3, task_callback, qOS::core::MEDIUM_PRIORITY, 2.0f, task::PERIODIC, qOS::ENABLED, nullptr );

    qOS::fsm::timeoutSpec_t tm_specTimeout;
    m.setup( nullptr, s1 );
    m.installTimeoutSpec( tm_specTimeout );
    m.add( s1, s1_callback );
    m.add( s2, s2_callback );
    qOS::os.addStateMachineTask( t4, m, qOS::core::MEDIUM_PRIORITY, 0.1f, qOS::ENABLED, nullptr );

    qOS::os.run();
    for(;;) { }

    return 0;
}