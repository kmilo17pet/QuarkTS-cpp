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

        _logger& out( const logSeverity s, const source_location &loc ) noexcept
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

        _logger& _logger::operator<<( const char& v )
        {
            writeChar( nullptr, v );
            return *this;
        }

        _logger& _logger::operator<<( const char * s )
        {
            (void)util::outputString( writeChar, s );
            return *this;
        }

        _logger& _logger::operator<<( const short& v )
        {
            return _log_integer( v, true );
        }

        _logger& _logger::operator<<( const int& v )
        {
            return _log_integer( v, true );
        }

        _logger& _logger::operator<<( const long int& v )
        {
            return _log_integer( v, true );
        }

        _logger& _logger::operator<<( const unsigned char& v )
        {
            return _log_integer( v, false );
        }

        _logger& _logger::operator<<( const unsigned short& v )
        {
            return _log_integer( v, false );
        }

        _logger& _logger::operator<<( const unsigned int& v )
        {
            return _logger::_log_integer( v, false );
        }

        _logger& _logger::operator<<( const unsigned long& v )
        {
            return _log_integer( v, false );
        }

        _logger& _logger::operator<<( const void * const p )
        {
            /*cstat -CERT-INT36-C*/
            (void)util::unsignedToString( reinterpret_cast<unsigned_t>( p ), buffer, 16U ); // skipcq: CXX-C1000
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
            return *this;
        }

        _logger& _logger::operator<<( const float64_t& v )
        {
            (void)util::floatToString( v, buffer, precision ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar, buffer ); // skipcq: CXX-C1000
            writeChar( nullptr, ' ' );
            return *this;
        }

        _logger& _logger::operator<<( const lout_base& f )
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
            return *this;
        }

        _logger& _logger::operator<<( const mem& m )
        {
            n = m.n;
            return *this;
        }

        _logger& _logger::operator<<( const pre& m )
        {
            precision = m.precision;
            return *this;
        }

        _logger& _logger::operator<<( const task& t )
        {
            (void)util::unsignedToString( t.getID(), buffer, 10 ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , "T{ \"" );
            (void)util::outputString( writeChar , t.getName() );
            (void)util::outputString( writeChar , "\", " );
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , ", " );
            (void)util::outputString( writeChar , ( taskState::ENABLED_STATE == t.getState() )? "enabled" : "disabled" );
            (void)util::outputString( writeChar , " } " );
            return *this;
        }

        _logger& _logger::operator<<( const qOS::timer& t )
        {
            (void)util::outputString( writeChar , "t{ E:" );
            (void)util::unsignedToString( t.elapsed(), buffer, 10 ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , ", R: " );
            (void)util::unsignedToString( t.remaining(), buffer, 10 ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , " } " );
            return *this;
        }
        /*cstat -CERT-INT36-C*/
        _logger& _logger::operator<<( const qOS::stateMachine& sm )
        {
            (void)util::outputString( writeChar , "SM{ T: 0x" );
            (void)util::unsignedToString( reinterpret_cast<unsigned_t>( &sm.getTop() ), buffer, 16 ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , ", C: 0x" );
            (void)util::unsignedToString( reinterpret_cast<unsigned_t>( sm.getCurrent() ), buffer, 16 ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , " } " );
            return *this;
        }

        _logger& _logger::operator<<( const qOS::sm::state& s )
        {
            (void)util::outputString( writeChar , "s{ 0x" );
            (void)util::unsignedToString( reinterpret_cast<unsigned_t>( &s ), buffer, 16 ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , buffer ); // skipcq: CXX-C1000
            (void)util::outputString( writeChar , " } " );
            return *this;
        }
        /*cstat +CERT-INT36-C*/
        #if defined( ARDUINO_PLATFORM )
        _logger& _logger::operator<<( const String & s )
        #else
        _logger& _logger::operator<<( const string & s )
        #endif
        {
            (void)util::outputString( writeChar, s.c_str() );
            return *this;
        }
    }

}
