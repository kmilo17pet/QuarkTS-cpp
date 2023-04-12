#include "include/trace.hpp"


namespace qOS {
    const char * const trace::endl = "\r\n";

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
    }

}

