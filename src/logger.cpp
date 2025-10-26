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
            //return _logger_out;
            return ChainLoggerProxy(_logger_out);
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

        void _logger::toLog( const short& v )
        {
            _log_integer( v, true );
        }

        void _logger::toLog( const int& v )
        {
            _log_integer( v, true );
        }

        void _logger::toLog( const long int& v )
        {
            _log_integer( v, true );
        }

        void _logger::toLog( const unsigned char& v )
        {
            _log_integer( v, false );
        }

        void _logger::toLog( const unsigned short& v )
        {
            _log_integer( v, false );
        }

        void _logger::toLog( const unsigned int& v )
        {
            _logger::_log_integer( v, false );
        }

        void _logger::toLog( const unsigned long& v )
        {
            _log_integer( v, false );
        }

        void _logger::toLog( const void * const p )
        {
            /*cstat -CERT-INT36-C*/
            (void)util::unsignedToString( reinterpret_cast<uintptr_t>( p ), buffer, 16U ); // skipcq: CXX-C1000
            /*cstat +CERT-INT36-C*/
            (void)util::outputString( writeChar, "p@0x" );
            (void)util::outputString( writeChar, buffer ); // skipcq: CXX-C1000
            if ( n > 0U ) {
                (void)util::outputString( writeChar, " = [ " );
                (void)util::printXData( writeChar, const_cast<void*>( p ), n, false );
                n  = 0U;
                writeChar( nullptr, ']' );
            }
            writeChar( nullptr, ' ' );
        }

        void _logger::toLog( const float64_t& v )
        {
            (void)util::floatToString( v, buffer, precision ); // skipcq: CXX-C1000
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

        void _logger::toLog( const task& t )
        {
            (void)util::unsignedToString( t.getID(), buffer, 10 ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , "T{ \"" );
            (void)util::outputString( writeChar , t.getName() );
            (void)util::outputString( writeChar , "\", " );
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , ", " );
            (void)util::outputString( writeChar , ( taskState::ENABLED_STATE == t.getState() )? "enabled" : "disabled" );
            (void)util::outputString( writeChar , " } " );
        }

        void _logger::toLog( const qOS::timer& t )
        {
            (void)util::outputString( writeChar , "t{ E:" );
            (void)util::unsignedToString( t.elapsed(), buffer, 10 ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , ", R: " );
            (void)util::unsignedToString( t.remaining(), buffer, 10 ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , " } " );
        }
        /*cstat -CERT-INT36-C*/
        void _logger::toLog( const qOS::stateMachine& sm )
        {
            (void)util::outputString( writeChar , "SM{ T: 0x" );
            (void)util::unsignedToString( reinterpret_cast<uintptr_t>( &sm.getTop() ), buffer, 16 ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , ", C: 0x" );
            (void)util::unsignedToString( reinterpret_cast<uintptr_t>( sm.getCurrent() ), buffer, 16 ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , " } " );
        }

        void _logger::toLog( const qOS::sm::state& s )
        {
            (void)util::outputString( writeChar , "s{ 0x" );
            (void)util::unsignedToString( reinterpret_cast<uintptr_t>( &s ), buffer, 16 ); // skipcq: CXX-C1000
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

        void _logger::toLog( const qOS::input::channel& in )
        {
            static const char *str[ static_cast<int>( qOS::input::event::MAX_EVENTS ) ] = { // skipcq: CXX-W2066
                "NONE ", "EXCEPTION ", "ON_CHANGE ", "FALLING_EDGE ", "RISING_EDGE ",
                "PULSATION_DOUBLE ", "PULSATION_TRIPLE ", "PULSATION_MULTI ",
                "HIGH_THRESHOLD ", "LOW_THRESHOLD ", "IN_BAND ", "STEADY_IN_HIGH ",
                "STEADY_IN_LOW ", "STEADY_IN_BAND ", "DELTA ", "STEP_UP ", "STEP_DOWN "
            };
            const auto e = in.getEvent();
            (void)util::outputString( writeChar , "in{ 0x" );
            (void)util::unsignedToString( reinterpret_cast<uintptr_t>( &in ), buffer, 16 ); // skipcq: CXX-C1000
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

        ChainLoggerProxy::~ChainLoggerProxy()
        {
            (void)util::outputString( parent.writeChar, logger::end );
        }
        ChainLoggerProxy& ChainLoggerProxy::operator<<( const char& v )
        {
            parent.toLog( v );
            return *this;
        }
        ChainLoggerProxy& ChainLoggerProxy::operator<<( const char * s )
        {
            parent.toLog( s );
            return *this;
        }
        ChainLoggerProxy& ChainLoggerProxy::operator<<( const short& v )
        {
            parent.toLog( v );
            return *this;
        }
        ChainLoggerProxy& ChainLoggerProxy::operator<<( const int& v )
        {
            parent.toLog( v );
            return *this;
        }
        ChainLoggerProxy& ChainLoggerProxy::operator<<( const long int& v )
        {
            parent.toLog( v );
            return *this;
        }
        ChainLoggerProxy& ChainLoggerProxy::operator<<( const unsigned char& v )
        {
            parent.toLog( v );
            return *this;
        }
        ChainLoggerProxy& ChainLoggerProxy::operator<<( const unsigned short& v )
        {
            parent.toLog( v );
            return *this;
        }
        ChainLoggerProxy& ChainLoggerProxy::operator<<( const unsigned int& v )
        {
            parent.toLog( v );
            return *this;
        }
        ChainLoggerProxy& ChainLoggerProxy::operator<<( const unsigned long& v )
        {
            parent.toLog( v );
            return *this;
        }
        ChainLoggerProxy& ChainLoggerProxy::operator<<( const void * const p )
        {
            parent.toLog( p );
            return *this;
        }
        ChainLoggerProxy& ChainLoggerProxy::operator<<( const float64_t& v )
        {
            parent.toLog( v );
            return *this;
        }
        ChainLoggerProxy& ChainLoggerProxy::operator<<( const lout_base& f )
        {
            parent.toLog( f );
            return *this;
        }
        ChainLoggerProxy& ChainLoggerProxy::operator<<( const mem& m )
        {
            parent.toLog( m );
            return *this;
        }
        ChainLoggerProxy& ChainLoggerProxy::operator<<( const pre& m )
        {
            parent.toLog( m );
            return *this;
        }
        ChainLoggerProxy& ChainLoggerProxy::operator<<( const task& t )
        {
            parent.toLog( t );
            return *this;
        }
        ChainLoggerProxy& ChainLoggerProxy::operator<<( const qOS::timer& t )
        {
            parent.toLog( t );
            return *this;
        }
        ChainLoggerProxy& ChainLoggerProxy::operator<<( const qOS::stateMachine& sm )
        {
            parent.toLog( sm );
            return *this;
        }
        ChainLoggerProxy& ChainLoggerProxy::operator<<( const qOS::sm::state& s )
        {
            parent.toLog( s );
            return *this;
        }
        ChainLoggerProxy& ChainLoggerProxy::operator<<( const qOS::trigger& t )
        {
            parent.toLog( t );
            return *this;
        }
        ChainLoggerProxy& ChainLoggerProxy::operator<<( const qOS::input::channel& in )
        {
            parent.toLog( in );
            return *this;
        }
        ChainLoggerProxy& ChainLoggerProxy::operator<<( const qOS::string & s )
        {
            parent.toLog( s );
            return *this;
        }

        /*cstat +CERT-INT36-C*/
        void _logger::toLog( const qOS::string & s )
        {
            (void)util::outputString( writeChar, s.c_str() );
        }
    }

}
