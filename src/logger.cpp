#include "include/logger.hpp"

namespace qOS {
    /*cstat -MISRAC++2008-0-1-4_b*/
    const char * const logger::endl = "\r\n";
    #if ( Q_LOGGER_COLORED == 1 )
        const char * const logger::end = "\x1B[0m\r\n";
        const char * const logger::nrm = "\x1B[0m";
        const char * const logger::red = "\x1B[31m";
        const char * const logger::grn = "\x1B[32m";
        const char * const logger::yel = "\x1B[33m";
        const char * const logger::blu = "\x1B[34m";
        const char * const logger::mag = "\x1B[35m";
        const char * const logger::cyn = "\x1B[36m";
        const char * const logger::wht = "\x1B[37m";
    #else
        const char * const logger::end = "\r\n";
        const char * const logger::nrm = "";
        const char * const logger::red = "";
        const char * const logger::grn = "";
        const char * const logger::yel = "";
        const char * const logger::blu = "";
        const char * const logger::mag = "";
        const char * const logger::cyn = "";
        const char * const logger::wht = "";
    #endif
    /*cstat +MISRAC++2008-0-1-4_b*/
    /* cppcheck-suppress noConstructor */
    namespace logger {
         _logger& _logger_out = _logger::getInstance(); // skipcq: CXX-W2011, CXX-W2009
        const lout_base dec( 10U );
        const lout_base hex( 16U );
        const lout_base oct( 8U );
        const lout_base bin( 2U );

        void setOutputFcn( util::putChar_t fcn )
        {
            if ( nullptr != fcn ) {
                _logger_out.writeChar = fcn;
            }
        }

        /*cstat -MISRAC++2008-0-1-7*/
        ChainLoggerProxy out( const logSeverity s, const source_location &loc ) noexcept
        {
            _logger_out.toLog( "[ " );
            _logger_out.toLog( dec );
            _logger_out.toLog( clock::getTick() );
            _logger_out.toLog( "] " );
            _logger_out.toLog( _logger_out.s_str[ s ] );

            if ( s == logSeverity::debug ) {
                _logger_out.toLog( "( " );
                _logger_out.toLog( loc.function_name() );
                _logger_out.toLog( ":" );
                _logger_out.toLog( loc.line() );
                _logger_out.toLog( "): " );
            }
            else if ( s == logSeverity::verbose ) {
                _logger_out.toLog( loc.file_name() );
                _logger_out.toLog( " ( " );
                _logger_out.toLog( loc.function_name() );
                _logger_out.toLog( ":" );
                _logger_out.toLog( loc.line() );
                _logger_out.toLog( "): " );
            }
            return ChainLoggerProxy(_logger_out); // skipcq: CXX-W2033
        }
        /*cstat +MISRAC++2008-0-1-7*/

        void _logger::writeNumStr( void ) noexcept
        {
            if ( '\0' != preFix[ 0 ] ) {
                (void)util::outputString( writeChar, preFix ); // skipcq: CXX-C1000
            }
            (void)util::outputString( writeChar, buffer ); // skipcq: CXX-C1000
            writeChar( nullptr, ' ' );
        }

        _logger& _logger::getInstance( void ) noexcept
        {
            static _logger instance;
            return instance;
        }

        void _logger::toLog( const char& v )
        {
            writeChar( nullptr, v );
        }

        void _logger::toLog( const char * s )
        {
            (void)util::outputString( writeChar, s );
        }

        void _logger::toLog( const void * const p )
        {
            (void)util::pointerToString( p, buffer );
            //(void)util::outputString( writeChar, "p@" );
            writeChar( nullptr, '@' );
            writeChar( nullptr, ':' );
            (void)util::outputString( writeChar, buffer ); // skipcq: CXX-C1000
            if ( n > 0U ) {
                (void)util::outputString( writeChar, " = [ " );
                (void)util::printXData( writeChar, const_cast<void*>( p ), n, false );
                n  = 0U;
                writeChar( nullptr, ']' );
            }
            writeChar( nullptr, ' ' );
        }

        void _logger::toLog( const float32_t& v )
        {
            (void)util::floatToString( v, buffer, precision ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar, buffer ); // skipcq: CXX-C1000
            writeChar( nullptr, ' ' );
        }

        void _logger::toLog( const float64_t& v )
        {
            (void)util::doubleToString( v, buffer, precision ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar, buffer ); // skipcq: CXX-C1000
            writeChar( nullptr, ' ' );
        }

        void _logger::toLog( const lout_base& f )
        {
            base = f.base;
            switch( f.base ) {
                case 2U:
                    (void)util::strcpy( preFix, "0b", sizeof(preFix) ); // skipcq: CXX-C1000
                    break;
                case 8U:
                    (void)util::strcpy( preFix, "0", sizeof(preFix) ); // skipcq: CXX-C1000
                    break;
                case 16U:
                    (void)util::strcpy( preFix, "0x", sizeof(preFix) ); // skipcq: CXX-C1000
                    break;
                default:
                    (void)memset( preFix, 0, sizeof(preFix) ); // skipcq: CXX-C1000
                    break;
            }
        }

        void _logger::toLog( const mem& m )
        {
            n = m.n;
        }

        void _logger::toLog( const pre& m )
        {
            precision = m.precision;
        }

        void _logger::toLog( const qOS::task& t )
        {
            (void)util::outputString( writeChar , "task{ " );
            const auto name = t.getName();
            if ( '\0' == name[ 0 ] ) {
                (void)util::pointerToString( &t, buffer );
                (void)util::outputString( writeChar, buffer );
            }
            else {
                (void)util::outputString( writeChar , t.getName() );
            }
            (void)util::outputString( writeChar , ", " );
            (void)util::unsignedToString( t.getID(), buffer, 10 ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , ", " );
            (void)util::outputString( writeChar , ( taskState::ENABLED_STATE == t.getState() )? "enabled" : "disabled" );
            (void)util::outputString( writeChar , " } " );
        }

        void _logger::toLog( const qOS::timer& t )
        {
            (void)util::outputString( writeChar , "timer{ E: " );
            (void)util::unsignedToString( t.elapsed(), buffer, 10 ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , ", R: " );
            (void)util::unsignedToString( t.remaining(), buffer, 10 ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , " } " );
        }

        void _logger::toLog( const qOS::stateMachine& sm )
        {
            (void)util::outputString( writeChar , "stateMachine{ T: " );
            (void)util::pointerToString( &sm.getTop(), buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , ", C: " );
            (void)util::pointerToString( sm.getCurrent(), buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , " } " );
        }

        void _logger::toLog( const qOS::sm::state& s )
        {
            (void)util::outputString( writeChar , "sm::state{ " );
            (void)util::pointerToString( &s, buffer );
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , " } " );
        }

        void _logger::toLog( const qOS::queue& v )
        {
            (void)util::outputString( writeChar , "queue{ H: " );
            (void)util::pointerToString( v.peek(), buffer );
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , ", count: " );
            (void)util::unsignedToString( v.count(), buffer, 10 );
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , " } " );
        }

        void _logger::toLog( const qOS::mem::pool& v )
        {
            (void)util::outputString( writeChar , "mem::pool{ " );
            (void)util::pointerToString( v.getPoolArea(), buffer );
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , ", size: " );
            (void)util::unsignedToString( v.getTotalSize(), buffer, 10 );
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , ", free: " );
            (void)util::unsignedToString( v.getFreeSize(), buffer, 10 );
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , " } " );
        }

        void _logger::toLog( const qOS::trigger& t )
        {
            static const char *str[ static_cast<int>( qOS::input::event::MAX_EVENTS ) + 1 ] = { // skipcq: CXX-W2066
                "None ", "byTimeElapsed ", "byNotificationQueued ", "byNotificationSimple ",
                "byQueueReceiver ", "byQueueFull ", "byQueueCount ", "byQueueEmpty ",
                "byEventFlags ", "bySchedulingRelease ", "byNoReadyTasks "
            };
            (void)util::outputString( writeChar , str[ static_cast<int>( t ) ] ); // skipcq: CXX-C1000
        }

        void _logger::toLog( const qOS::globalState& s )
        {
            static const char *str[ 5 ] = { // skipcq: CXX-W2066
                "UNDEFINED ", "READY ", "WAITING ", "SUSPENDED ", "RUNNING "
            };
            (void)util::outputString( writeChar , str[ static_cast<int>( s ) ] ); // skipcq: CXX-C1000
        }

        void _logger::toLog( const qOS::taskEvent& e )
        {
            toLog( e.thisTask() );
            (void)util::outputString( writeChar , "-> event_t{ " );
            if ( e.firstCall() ) {
                (void)util::outputString( writeChar , "firstCall, " );
            }
            if ( e.firstIteration() ) {
                (void)util::outputString( writeChar , "firstIteration, " );
            }
            if ( e.lastIteration() ) {
                (void)util::outputString( writeChar , "lastIteration, " );
            }
            (void)util::outputString( writeChar , "startDelay: " );
            (void)util::unsignedToString( e.startDelay(), buffer, 10 ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , ", trigger: " );
            toLog( e.getTrigger() );
            (void)util::outputString( writeChar , "} " );
        }

        void _logger::toLog( const qOS::taskState& v )
        {
            static const char *str[ 4 ] = { // skipcq: CXX-W2066
                "DISABLED ", "ENABLED ", "AWAKE ", "ASLEEP "
            };
            (void)util::outputString( writeChar , str[ static_cast<int>( v ) ] ); // skipcq: CXX-C1000
        }

        void _logger::toLog( const qOS::input::channel& in )
        {
            static const char *str[ static_cast<int>( qOS::input::event::MAX_EVENTS ) ] = { // skipcq: CXX-W2066
                "NONE ", "EXCEPTION ", "ON_CHANGE ", "FALLING_EDGE ", "RISING_EDGE ",
                "PULSATION_DOUBLE ", "PULSATION_TRIPLE ", "PULSATION_MULTI ",
                "HIGH_THRESHOLD ", "LOW_THRESHOLD ", "IN_BAND ", "STEADY_IN_HIGH ",
                "STEADY_IN_LOW ", "STEADY_IN_BAND ", "DELTA ", "STEP_UP ", "STEP_DOWN "
            };
            const auto e = in.getEvent();
            (void)util::outputString( writeChar , "input::channel{ " );
            (void)util::pointerToString( &in, buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , ( qOS::input::type::ANALOG_CHANNEL == in.getType() ) ? ", ANALOG, C: " : ", DIGITAL, C: " );
            /*cstat -MISRAC++2008-5-0-8*/
            (void)util::unsignedToString( static_cast<unsigned_t>( in.getChannel() ), buffer, 10 ); // skipcq: CXX-C1000
            /*cstat +MISRAC++2008-5-0-8*/
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , " } " );
            if ( qOS::input::event::NONE != e ) {
                (void)util::outputString( writeChar , str[ static_cast<int>( e ) ] ); // skipcq: CXX-C1000
            }
        }

        void _logger::toLog( const qOS::input::watcher& v )
        {
            (void)util::outputString( writeChar , "input::watcher{ " );
            (void)util::pointerToString( &v, buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , ", ANALOG: " );
            (void)util::unsignedToString( v.getAnalogChannelsCount(), buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , ", DIGITAL: " );
            (void)util::unsignedToString( v.getDigitalChannelsCount(), buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , " } " );
        }

        void _logger::toLog( const qOS::sm::signalID & v )
        {
            const char *str = nullptr;
            buffer[ 0 ] = ' ';
            buffer[ 1 ] = '\0';

            switch ( v ) {
                case qOS::sm::SIGNAL_START:  str = "SIGNAL_START";  break;
                case qOS::sm::SIGNAL_EXIT:   str = "SIGNAL_EXIT";   break;
                case qOS::sm::SIGNAL_ENTRY:  str = "SIGNAL_ENTRY";  break;
                case qOS::sm::SIGNAL_NONE:   str = "SIGNAL_NONE";   break;
                default:
                    break;
            }
            if ( str != nullptr ) {
                (void)util::outputString( writeChar, str );
                (void)util::outputString( writeChar, buffer );
            }
            else {
                const unsigned sig  = static_cast<unsigned>( v );
                if ( ( v >= qOS::sm::TM_MIN ) && ( v <= qOS::sm::TM_MAX  ) ) {
                    const unsigned tBase = static_cast<unsigned>( qOS::sm::SIGNAL_TIMEOUT0 );
                    const unsigned maxIdx = static_cast<unsigned>( Q_FSM_MAX_TIMEOUTS) - 1U;
                    const unsigned idx = maxIdx - ( tBase - sig );
                    (void)util::outputString( writeChar, "SIGNAL_TIMEOUT" );
                    (void)util::unsignedToString( idx, buffer, 10 );
                    (void)util::outputString( writeChar, buffer );
                    writeChar( nullptr, ' ' );
                }
                else {
                    (void)util::outputString( writeChar, "SIGNAL_USER:" );
                    (void)util::unsignedToString( sig, buffer, 10 );
                    (void)util::outputString( writeChar, buffer );
                    writeChar( nullptr, ' ' );
                }
            }
        }

        void _logger::toLog( const qOS::sm::status & v )
        {
            const char *str = "STATUS_USER:";
            buffer[ 0 ] = ' ';
            buffer[ 1 ] = '\0';

            switch ( v ) {
                case qOS::sm::status::BEFORE_ANY:
                    str = "BEFORE_ANY";
                    break;
                case qOS::sm::status::ABSENT:
                    str = "ABSENT";
                    break;
                case qOS::sm::status::FAILURE:
                    str = "FAILURE";
                    break;
                case qOS::sm::status::SUCCESS:
                    str = "SUCCESS";
                    break;
                case qOS::sm::status::SIGNAL_HANDLED:
                    str = "SIGNAL_HANDLED";
                    break;
                default:
                    /*cstat -MISRAC++2008-5-0-8*/
                    (void)util::unsignedToString( static_cast<unsigned_t>( v ), buffer, 10 ); // skipcq: CXX-C1000
                    /*cstat +MISRAC++2008-5-0-8*/
                    break;
            }
            (void)util::outputString( writeChar, str );
            (void)util::outputString( writeChar, buffer );
        }

        void _logger::toLog( const qOS::sm::stateHandler& v )
        {
            const auto &m = v.thisMachine();
            const auto &s = v.thisState();
            (void)util::outputString( writeChar , "sm::handler_t{ " );
            (void)util::pointerToString( &m, buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , " | state: " );
            (void)util::pointerToString( &s, buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , ", " );
            toLog( v.signal() );
            if ( qOS::sm::status::ABSENT != v.lastStatus() ) {
                (void)util::outputString( writeChar , ", lastStatus:" );
                toLog( v.lastStatus() );
            }
            (void)util::outputString( writeChar , "} " );
        }

        void _logger::toLog( const qOS::string & s )
        {
            (void)util::outputString( writeChar, s.c_str() );
        }

        ChainLoggerProxy::~ChainLoggerProxy()
        {
            (void)util::outputString( parent.writeChar, logger::end );
        }

    }

}
