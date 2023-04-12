#ifndef QOS_CPP_TRACE
#define QOS_CPP_TRACE

#include "include/types.hpp"
#include "include/util.hpp"
#include "include/clock.hpp"
#include "include/task.hpp"

namespace qOS {
    namespace trace {

        class tout_base {
            public:
                uint8_t base;
                tout_base(uint8_t b) : base(b) {}
        };

        extern const tout_base dec;
        extern const tout_base hex;
        extern const tout_base oct;
        extern const tout_base bin;
        extern const char * const endl;

        class _trace final {
            private:
                _trace() = default;
                _trace( _trace &other ) = delete;
                void operator=( const _trace & ) = delete;
            public:
                char buffer[64] = {0};
                uint8_t base = { 10u };
                util::putChar_t writeChar{ nullptr };
                static _trace& getInstance( void ) noexcept;
           
            friend _trace& operator<<( _trace& tout, const char c );
            friend _trace& operator<<( _trace& tout, const char * s );
            friend _trace& operator<<( _trace& tout, const int32_t& v );
            friend _trace& operator<<( _trace& tout, const uint32_t& v );
            friend _trace& operator<<( _trace& tout, const tout_base& f );

            friend _trace& operator<<( _trace& tout, const task& t );
        };

        extern _trace& _trace_out;

        inline void setOutputFcn( util::putChar_t fcn )
        {
            if ( nullptr != fcn ) {
                _trace_out.writeChar = fcn;
            }
        }
        inline void log(void) {}
        inline void msg(void) {}
        inline const char * var( const char * vname ){ return vname; }
    }
}

#define _TRACE_STRINGIFY(x)              #x
#define _TRACE_TOSTRING(x)               _TRACE_STRINGIFY( x )

#define var(v)  var( _TRACE_STRINGIFY(v) ) << '=' << v
#define log     log();qOS::trace::_trace_out<< '[' << qOS::trace::dec << qOS::clock::getTick() << "] " << __func__ << ":" _TRACE_TOSTRING(__LINE__) " - "
#define msg     msg();qOS::trace::_trace_out



#endif /*QOS_CPP_TRACE*/
