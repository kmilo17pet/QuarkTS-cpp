#ifndef QOS_CPP_TRACE
#define QOS_CPP_TRACE

#include "include/types.hpp"
#include "include/util.hpp"
#include "include/clock.hpp"
#include "include/task.hpp"
#include "include/timer.hpp"

#define _TRACE_STRINGIFY(x)              #x
#define _TRACE_TOSTRING(x)               _TRACE_STRINGIFY( x )

#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
    #define _TRACE_CURRENT_FUNCTION __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
    #define _TRACE_CURRENT_FUNCTION __PRETTY_FUNCTION__
#elif defined(__FUNCSIG__)
    #define _TRACE_CURRENT_FUNCTION __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
    #define _TRACE_CURRENT_FUNCTION __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
    #define _TRACE_CURRENT_FUNCTION __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
    #define _TRACE_CURRENT_FUNCTION __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
    #define _TRACE_CURRENT_FUNCTION __func__
#else
    #define _TRACE_CURRENT_FUNCTION "(unknown)"
#endif

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
        extern const char * const nrm;
        extern const char * const red;
        extern const char * const grn;
        extern const char * const yel;
        extern const char * const blu;
        extern const char * const mag;
        extern const char * const cyn;
        extern const char * const wht;

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
            friend _trace& operator<<( _trace& tout, const void * const p );
            friend _trace& operator<<( _trace& tout, const float32_t& v );
            friend _trace& operator<<( _trace& tout, const tout_base& f );

            friend _trace& operator<<( _trace& tout, const task& t );
            friend _trace& operator<<( _trace& tout, const qOS::timer& t );
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

#define var(v)  var( _TRACE_STRINGIFY(v) ) << '=' << v
#define log     log();qOS::trace::_trace_out<< '[' << qOS::trace::dec << qOS::clock::getTick() << "] " << _TRACE_CURRENT_FUNCTION << ":" _TRACE_TOSTRING(__LINE__) " - "
#define msg     msg();qOS::trace::_trace_out

#endif /*QOS_CPP_TRACE*/
