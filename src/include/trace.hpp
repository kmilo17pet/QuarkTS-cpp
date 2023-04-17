#ifndef QOS_CPP_TRACE
#define QOS_CPP_TRACE

#include "include/types.hpp"
#include "include/util.hpp"
#include "include/clock.hpp"
#include "include/task.hpp"
#include "include/fsm.hpp"
#include "include/timer.hpp"

#if defined( ARDUINO_ARCH_AVR) || defined( ARDUINO_ARCH_SAMD )
    #include <Arduino.h>
#else
    #include <string>
#endif


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

        class mem {
            public:
                size_t n;
                mem( size_t nb ) : n( nb ) {}
        };

        class pre {
            public:
                uint8_t precision;
                pre( uint8_t p ) : precision( p ) {}
        };

        extern const tout_base dec;
        extern const tout_base hex;
        extern const tout_base oct;
        extern const tout_base bin;
        extern const char * const endl;
        extern const char * const end;
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

                uint8_t base = { 10u };
                size_t n{ 0u };
                uint8_t precision { 6u };
                char buffer[ 21 ] = { 0 };
                char preFix[ 5 ] = { 0 };
                util::putChar_t writeChar{ nullptr };
            public:
                static _trace& getInstance( void ) noexcept;
            friend void setOutputFcn( util::putChar_t fcn );
            friend _trace& operator<<( _trace& tout, const char c );
            friend _trace& operator<<( _trace& tout, const char * s );
            friend _trace& operator<<( _trace& tout, const int32_t& v );
            #if ULONG_MAX > UINT32_MAX
                friend _trace& operator<<( _trace& tout, const uint32_t& v );
            #endif
            friend _trace& operator<<( _trace& tout, const unsigned_t& v );
            
            friend _trace& operator<<( _trace& tout, const void * const p );
            friend _trace& operator<<( _trace& tout, const float64_t& v );
            friend _trace& operator<<( _trace& tout, const tout_base& f );
            friend _trace& operator<<( _trace& tout, const mem& m );
            friend _trace& operator<<( _trace& tout, const pre& m );

            friend _trace& operator<<( _trace& tout, const task& t );
            friend _trace& operator<<( _trace& tout, const qOS::timer& t );
            friend _trace& operator<<( _trace& tout, const qOS::stateMachine& sm );
            friend _trace& operator<<( _trace& tout, const qOS::sm::state& s );

            #if defined( ARDUINO_ARCH_AVR) || defined( ARDUINO_ARCH_SAMD )
                friend _trace& operator<<( _trace& tout, const String & s );
            #else
                friend _trace& operator<<( _trace& tout, const string & s );
            #endif
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

#define _logHeader()                                                           \
qOS::trace::_trace_out << "[ " << qOS::trace::dec <<                           \
static_cast<unsigned long>( qOS::clock::getTick() )                            \
<< "] " << _TRACE_CURRENT_FUNCTION << ":" _TRACE_TOSTRING(__LINE__) " - "      \

#define var(v)  var( _TRACE_STRINGIFY(v) ) << '=' << v
#define log     log();_logHeader()
#define msg     msg();qOS::trace::_trace_out

#endif /*QOS_CPP_TRACE*/
