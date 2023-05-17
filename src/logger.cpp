#include "include/logger.hpp"

namespace qOS {
    /*cstat -MISRAC++2008-0-1-4_b*/
    const char * const logger::endl = "\r\n";
    const char * const logger::end = "\x1B[0m\r\n";
    const char * const logger::nrm = "\x1B[0m";
    const char * const logger::red = "\x1B[31m";
    const char * const logger::grn = "\x1B[32m";
    const char * const logger::yel = "\x1B[33m";
    const char * const logger::blu = "\x1B[34m";
    const char * const logger::mag = "\x1B[35m";
    const char * const logger::cyn = "\x1B[36m";
    const char * const logger::wht = "\x1B[37m";
    /*cstat +MISRAC++2008-0-1-4_b*/
    /* cppcheck-suppress noConstructor */
    namespace logger {
         _logger& _logger_out = _logger::getInstance(); // skipcq: CXX-W2011
        const lout_base dec( 10u );
        const lout_base hex( 16u );
        const lout_base oct( 8u );
        const lout_base bin( 2u );

        void setOutputFcn( util::putChar_t fcn )
        {
            if ( nullptr != fcn ) {
                _logger_out.writeChar = fcn;
            }
        }

        _logger& out( const logSeverity s, const source_location &loc )
        {
            _logger_out << "[ " <<  dec << clock::getTick() << "] " << _logger_out.s_str[ s ];
            if ( s == logSeverity::debug ) {
                _logger_out << "( " << loc.function_name() << ":" << loc.line() << "): ";
            }
            else if ( s == logSeverity::verbose ) {
                _logger_out << 
                loc.file_name() << " ( " << loc.function_name() << ":" << loc.line() << "): ";
            }
            return _logger_out;
        }

        _logger& _logger::getInstance( void ) noexcept
        {
            static _logger instance;
            return instance;
        }

        _logger& operator<<( _logger& tout, const char& c )
        {
            tout.writeChar( nullptr, c );
            return tout;
        }

        _logger& operator<<( _logger& tout, const char * s )
        {
            (void)util::outputString( tout.writeChar, s );
            return tout;
        }

        _logger& operator<<( _logger& tout, const short& v )
        {
            return _log_integer( tout, v, true );
        }

        _logger& operator<<( _logger& tout, const int& v )
        {
            return _log_integer( tout, v, true );
        }

        _logger& operator<<( _logger& tout, const long int& v )
        {
            return _log_integer( tout, v, true );
        }

        _logger& operator<<( _logger& tout, const unsigned char& v )
        {
            return _log_integer( tout, v, false );
        }

        _logger& operator<<( _logger& tout, const unsigned short& v )
        {
            return _log_integer( tout, v, false );
        }

        _logger& operator<<( _logger& tout, const unsigned int& v )
        {
            return _log_integer( tout, v, false );
        }

        _logger& operator<<( _logger& tout, const unsigned long& v )
        {
            return _log_integer( tout, v, false );
        }

        _logger& operator<<( _logger& tout, const void * const p )
        {
            /*cstat -CERT-INT36-C*/
            (void)util::unsignedToString( reinterpret_cast<unsigned_t>( p ), tout.buffer, 16u ); // skipcq: CXX-C1000
            /*cstat +CERT-INT36-C*/
            (void)util::outputString( tout.writeChar, "p@0x" );
            (void)util::outputString( tout.writeChar, tout.buffer ); // skipcq: CXX-C1000
            if ( tout.n > 0u ) {
                (void)util::outputString( tout.writeChar, " = [ " );
                (void)util::printXData( tout.writeChar, const_cast<void*>( p ), tout.n, false );
                tout.n  = 0u;
                tout.writeChar( nullptr, ']' );
            }
            tout.writeChar( nullptr, ' ' );
            return tout;
        }

        _logger& operator<<( _logger& tout, const float64_t& v )
        {
            (void)util::floatToString( v, tout.buffer, tout.precision ); // skipcq: CXX-C1000
            (void)util::outputString( tout.writeChar, tout.buffer ); // skipcq: CXX-C1000
            tout.writeChar( nullptr, ' ' );
            return tout;
        }

        _logger& operator<<( _logger& lout, const lout_base& f )
        {
            lout.base = f.base;
            switch( f.base ) {
                case 2u:
                    (void)util::strcpy( lout.preFix, "0b", sizeof(lout.preFix) ); // skipcq: CXX-C1000
                    break;
                case 8u:
                    (void)util::strcpy( lout.preFix, "0", sizeof(lout.preFix) ); // skipcq: CXX-C1000
                    break;
                case 16u:
                    (void)util::strcpy( lout.preFix, "0x", sizeof(lout.preFix) ); // skipcq: CXX-C1000
                    break;
                default:
                    (void)memset( lout.preFix, 0, sizeof(lout.preFix) ); // skipcq: CXX-C1000
                    break;
            }
            return lout;
        }

        _logger& operator<<( _logger& tout, const mem& m )
        {
            tout.n = m.n;
            return tout;
        }

        _logger& operator<<( _logger& tout, const pre& m )
        {
            tout.precision = m.precision;
            return tout;
        }

        _logger& operator<<( _logger& tout, const task& t )
        {
            (void)util::unsignedToString( t.getID(), tout.buffer, 10 ); // skipcq: CXX-C1000
            (void)util::outputString( tout.writeChar , "T{ \"" );
            (void)util::outputString( tout.writeChar , t.getName() );
            (void)util::outputString( tout.writeChar , "\", " );
            (void)util::outputString( tout.writeChar , tout.buffer ); // skipcq: CXX-C1000
            (void)util::outputString( tout.writeChar , ", " );
            (void)util::outputString( tout.writeChar , ( taskState::ENABLED_STATE == t.getState() )? "enabled" : "disabled" );           
            (void)util::outputString( tout.writeChar , " } " );
            return tout;
        }

        _logger& operator<<( _logger& tout, const qOS::timer& t )
        {
            (void)util::outputString( tout.writeChar , "t{ E:" );
            (void)util::unsignedToString( t.elapsed(), tout.buffer, 10 ); // skipcq: CXX-C1000
            (void)util::outputString( tout.writeChar , tout.buffer ); // skipcq: CXX-C1000
            (void)util::outputString( tout.writeChar , ", R: " );
            (void)util::unsignedToString( t.remaining(), tout.buffer, 10 ); // skipcq: CXX-C1000
            (void)util::outputString( tout.writeChar , tout.buffer ); // skipcq: CXX-C1000
            (void)util::outputString( tout.writeChar , " } " );
            return tout;
        }
        /*cstat -CERT-INT36-C*/
        _logger& operator<<( _logger& tout, const qOS::stateMachine& sm )
        {
            (void)util::outputString( tout.writeChar , "SM{ T: 0x" );
            (void)util::unsignedToString( reinterpret_cast<unsigned_t>( &sm.getTop() ), tout.buffer, 16 ); // skipcq: CXX-C1000
            (void)util::outputString( tout.writeChar , tout.buffer ); // skipcq: CXX-C1000
            (void)util::outputString( tout.writeChar , ", C: 0x" );
            (void)util::unsignedToString( reinterpret_cast<unsigned_t>( sm.getCurrent() ), tout.buffer, 16 ); // skipcq: CXX-C1000
            (void)util::outputString( tout.writeChar , tout.buffer ); // skipcq: CXX-C1000
            (void)util::outputString( tout.writeChar , " } " );
            return tout;
        }

        _logger& operator<<( _logger& tout, const qOS::sm::state& s )
        {
            (void)util::outputString( tout.writeChar , "s{ 0x" );
            (void)util::unsignedToString( reinterpret_cast<unsigned_t>( &s ), tout.buffer, 16 ); // skipcq: CXX-C1000
            (void)util::outputString( tout.writeChar , tout.buffer ); // skipcq: CXX-C1000
            (void)util::outputString( tout.writeChar , " } " );
            return tout;
        }
        /*cstat +CERT-INT36-C*/
        #if defined( ARDUINO_PLATFORM )
        _logger& operator<<( _logger& tout, const String & s )
        #else
        _logger& operator<<( _logger& tout, const string & s )
        #endif
        {
            (void)util::outputString( tout.writeChar, s.c_str() );
            return tout;
        }
    }

}

