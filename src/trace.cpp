#include "include/trace.hpp"

namespace qOS {
    /*cstat -MISRAC++2008-0-1-4_b*/
    const char * const trace::endl = "\r\n";
    const char * const trace::end = "\x1B[0m\r\n";
    const char * const trace::nrm = "\x1B[0m";
    const char * const trace::red = "\x1B[31m";
    const char * const trace::grn = "\x1B[32m";
    const char * const trace::yel = "\x1B[33m";
    const char * const trace::blu = "\x1B[34m";
    const char * const trace::mag = "\x1B[35m";
    const char * const trace::cyn = "\x1B[36m";
    const char * const trace::wht = "\x1B[37m";
    /*cstat +MISRAC++2008-0-1-4_b*/
    namespace trace {
         _trace& _trace_out = _trace::getInstance(); // skipcq: CXX-W2011
        const tout_base dec( 10u );
        const tout_base hex( 16u );
        const tout_base oct( 8u );
        const tout_base bin( 2u );

        _trace& _trace::getInstance( void ) noexcept
        {
            static _trace instance;
            return instance;
        }

        _trace& operator<<( _trace& tout, const char c )
        {
            tout.writeChar( nullptr, c );
            return tout;
        }

        _trace& operator<<( _trace& tout, const char * s )
        {
            (void)util::outputString( tout.writeChar, s );
            return tout;
        }

        _trace& operator<<( _trace& tout, const int32_t& v )
        {
            (void)util::integerToString( static_cast<int32_t>( v ), tout.buffer, tout.base );
            if ( '\0' != tout.preFix[ 0 ] ) {
                (void)util::outputString( tout.writeChar, tout.preFix );
            } 
            (void)util::outputString( tout.writeChar, tout.buffer );
            tout.writeChar( nullptr, ' ' );
            return tout;
        }
        #if ULONG_MAX > UINT32_MAX
        _trace& operator<<( _trace& tout, const uint32_t& v )
        {
            (void)util::unsignedToString( static_cast<unsigned_t>( v ), tout.buffer, tout.base );
            if ( '\0' != tout.preFix[ 0 ] ) {
                (void)util::outputString( tout.writeChar, tout.preFix );
            } 
            (void)util::outputString( tout.writeChar, tout.buffer );
            tout.writeChar( nullptr, ' ' );
            return tout;
        }
        #endif
        _trace& operator<<( _trace& tout, const unsigned_t& v )
        {
            (void)util::unsignedToString( v, tout.buffer, tout.base );
            if ( '\0' != tout.preFix[ 0 ] ) {
                (void)util::outputString( tout.writeChar, tout.preFix );
            } 
            (void)util::outputString( tout.writeChar, tout.buffer );
            tout.writeChar( nullptr, ' ' );
            return tout;
        }

        _trace& operator<<( _trace& tout, const void * const p )
        {
            /*cstat -CERT-INT36-C*/
            (void)util::unsignedToString( reinterpret_cast<unsigned_t>( p ), tout.buffer, 10u );
            /*cstat +CERT-INT36-C*/
            (void)util::outputString( tout.writeChar, "p@0x" );
            (void)util::outputString( tout.writeChar, tout.buffer );
            if ( tout.n > 0u ) {
                (void)util::outputString( tout.writeChar, " = [ " );
                (void)util::printXData( tout.writeChar, const_cast<void*>( p ), tout.n, false );
                tout.n  = 0u;
                tout.writeChar( nullptr, ']' );
            }
            tout.writeChar( nullptr, ' ' );
            return tout;
        }

        _trace& operator<<( _trace& tout, const float64_t& v )
        {
            (void)util::floatToString( v, tout.buffer, tout.precision );
            (void)util::outputString( tout.writeChar, tout.buffer );
            tout.writeChar( nullptr, ' ' );
            return tout;
        }

        _trace& operator<<( _trace& tout, const tout_base& f )
        {
            tout.base = f.base;
            switch( f.base ) {
                case 2u:
                    (void)util::strcpy( tout.preFix, "0b", sizeof(tout.preFix) );
                    break;
                case 8u:
                    (void)util::strcpy( tout.preFix, "0", sizeof(tout.preFix) );
                    break;
                case 16u:
                    (void)util::strcpy( tout.preFix, "0x", sizeof(tout.preFix) );
                    break;
                default:
                    (void)memset( tout.preFix, 0, sizeof(tout.preFix) );
                    break;
            }
            return tout;
        }

        _trace& operator<<( _trace& tout, const mem& m )
        {
            tout.n = m.n;
            return tout;
        }

        _trace& operator<<( _trace& tout, const pre& m )
        {
            tout.precision = m.precision;
            return tout;
        }

        _trace& operator<<( _trace& tout, const task& t )
        {
            (void)util::unsignedToString( t.getID(), tout.buffer, 10 );
            (void)util::outputString( tout.writeChar , "T{ \"" );
            (void)util::outputString( tout.writeChar , t.getName() );
            (void)util::outputString( tout.writeChar , "\", " );
            (void)util::outputString( tout.writeChar , tout.buffer );
            (void)util::outputString( tout.writeChar , ", " );
            switch ( t.getState() ) {
                case taskState::DISABLED_STATE:
                    (void)util::outputString( tout.writeChar , "disabled" );
                    break;
                case taskState::ENABLED_STATE:
                    (void)util::outputString( tout.writeChar , "enabled" );
                    break;
                case taskState::AWAKE_STATE:
                    (void)util::outputString( tout.writeChar , "awake" );
                    break;
                case taskState::ASLEEP_STATE:
                    (void)util::outputString( tout.writeChar , "asleep" );
                    break;
                default:
                    break;
            }
            
            (void)util::outputString( tout.writeChar , " } " );
            return tout;
        }

        _trace& operator<<( _trace& tout, const qOS::timer& t )
        {
            (void)util::outputString( tout.writeChar , "t{ E:" );
            (void)util::unsignedToString( t.elapsed(), tout.buffer, 10 );
            (void)util::outputString( tout.writeChar , tout.buffer );
            (void)util::outputString( tout.writeChar , ", R: " );
            (void)util::unsignedToString( t.remaining(), tout.buffer, 10 );
            (void)util::outputString( tout.writeChar , tout.buffer );
            (void)util::outputString( tout.writeChar , " } " );
            return tout;
        }
        /*cstat -CERT-INT36-C*/
        _trace& operator<<( _trace& tout, const qOS::stateMachine& sm )
        {
            (void)util::outputString( tout.writeChar , "SM{ T: 0x" );
            (void)util::unsignedToString( reinterpret_cast<unsigned_t>( &sm.getTop() ), tout.buffer, 16 );
            (void)util::outputString( tout.writeChar , tout.buffer );
            (void)util::outputString( tout.writeChar , ", C: 0x" );
            (void)util::unsignedToString( reinterpret_cast<unsigned_t>( sm.getCurrent() ), tout.buffer, 16 );
            (void)util::outputString( tout.writeChar , tout.buffer );
            (void)util::outputString( tout.writeChar , " } " );
            return tout;
        }

        _trace& operator<<( _trace& tout, const qOS::sm::state& s )
        {
            (void)util::outputString( tout.writeChar , "s{ 0x" );
            (void)util::unsignedToString( reinterpret_cast<unsigned_t>( &s ), tout.buffer, 16 );
            (void)util::outputString( tout.writeChar , tout.buffer );
            (void)util::outputString( tout.writeChar , " } " );
            return tout;
        }
        /*cstat +CERT-INT36-C*/
        #if defined( ARDUINO_PLATFORM )
        _trace& operator<<( _trace& tout, const String & s )
        #else
        _trace& operator<<( _trace& tout, const string & s )
        #endif
        {
            (void)util::outputString( tout.writeChar, s.c_str() );
            return tout;
        } 
    }

}

