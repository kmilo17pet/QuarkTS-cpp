#include "include/trace.hpp"


namespace qOS {
    const char * const trace::endl = "\r\n";
    const char * const trace::nrm = "\x1B[0m";
    const char * const trace::red = "\x1B[31m";
    const char * const trace::grn = "\x1B[32m";
    const char * const trace::yel = "\x1B[33m";
    const char * const trace::blu = "\x1B[34m";
    const char * const trace::mag = "\x1B[35m";
    const char * const trace::cyn = "\x1B[36m";
    const char * const trace::wht = "\x1B[37m";
    namespace trace {
        _trace& _trace_out = _trace::getInstance();
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
            (void)util::outputString( tout.writeChar, tout.buffer );
            return tout;
        }

        _trace& operator<<( _trace& tout, const uint32_t& v )
        {
            (void)util::unsignedToString( v, tout.buffer, tout.base );
            (void)util::outputString( tout.writeChar, tout.buffer );
            return tout;
        }

        _trace& operator<<( _trace& tout, const void * const p )
        {
            (void)util::unsignedToString( reinterpret_cast<size_t>( p ), tout.buffer, 10u );
            (void)util::outputString( tout.writeChar, "p@0x" );
            (void)util::outputString( tout.writeChar, tout.buffer );
            return tout;
        }

        _trace& operator<<( _trace& tout, const float32_t& v )
        {
            (void)util::floatToString( v, tout.buffer );
            (void)util::outputString( tout.writeChar, tout.buffer );
            return tout;
        }

        _trace& operator<<( _trace& tout, const tout_base& f )
        {
            tout.base = f.base;
            return tout;
        }

        _trace& operator<<( _trace& tout, const task& t )
        {
            const char *state[ 4 ] = {"disabled","enabled","awake","asleep"};
            (void)util::unsignedToString( t.getID(), tout.buffer, 10 );
            (void)util::outputString( tout.writeChar , "T{ \"" );
            (void)util::outputString( tout.writeChar , t.getName() );
            (void)util::outputString( tout.writeChar , "\", " );
            (void)util::outputString( tout.writeChar , tout.buffer );
            (void)util::outputString( tout.writeChar , ", " );
            (void)util::outputString( tout.writeChar , state[ t.getState() ]);
            (void)util::outputString( tout.writeChar , " } " );
            return tout;
        }

        _trace& operator<<( _trace& tout, const qOS::timer& t )
        {
            (void)util::outputString( tout.writeChar , "t{ E:" );
            (void)util::unsignedToString( t.elapsed(), tout.buffer, 10 );
            (void)util::outputString( tout.writeChar , tout.buffer );
            (void)util::outputString( tout.writeChar , "\", R: " );
            (void)util::unsignedToString( t.remaining(), tout.buffer, 10 );
            (void)util::outputString( tout.writeChar , tout.buffer );
            (void)util::outputString( tout.writeChar , " } " );
            return tout;
        }
    }

}

