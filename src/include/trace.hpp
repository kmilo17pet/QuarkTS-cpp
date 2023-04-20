#ifndef QOS_CPP_TRACE
#define QOS_CPP_TRACE

#include "include/types.hpp"
#include "include/util.hpp"
#include "include/clock.hpp"
#include "include/task.hpp"
#include "include/fsm.hpp"
#include "include/timer.hpp"

#if defined( ARDUINO_PLATFORM )
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

    /** @addtogroup qtrace
    * @brief API interfaces to print out trace and debug messages.
    *  @{
    */
    
    #ifdef DOXYGEN
        /**
        * @brief The global class to output trace streams. Its usage requires
        * one the following static methods: trace::log() or trace::msg()
        */
        class trace {
            public:
                /**
                * @brief Specify a new trace output with detailed information of the
                * caller.
                * @note Should be used only at the beginning of trace stream;
                * Example:
                * @code{.c}
                * trace::log << "some message" << trace::endl;
                * @endcode
                */
                static void* log;
                /**
                * @brief Specify a new trace output without detailed information of the 
                * caller.
                * @note Should be used only at the beginning of trace stream;
                * Example:
                * @code{.c}
                * trace::msg << "some message" << trace::endl;
                * @endcode
                */
                static void* msg;
                /**
                * @brief Specify that the variable given by @a v should be printed
                * with its own name  :  <var::name> = <var::value>
                * @param[in] v variable to be traced
                * @code{.c}
                * int myVariable;
                * trace::log << trace::var( myVariable ) << trace::endl;
                * @endcode
                */
                static void var( const void &v );
        };
    #endif

    namespace trace {

        /** @addtogroup qtrace
        *  @{
        */


        /*! @cond */
        class tout_base {
            public:
                uint8_t base;
                tout_base(uint8_t b) : base(b) {}
        };
        /*! @endcond */

        /**
        * @brief Class that sets the number of bytes to be traced when a pointer
        * is being used after.
        * Example:
        * @code{.c}
        * uin32_t myNumber = 0xAABBCCDD;
        * trace::log << trace::mem( sizeof(myNumber ) ) << &myNumber << trace::endl;
        * @endcode
        */
        class mem {
            public:
                size_t n;
                mem( size_t nb ) : n( nb ) {}
        };

        /**
        * @brief Class that sets the decimal precision to be used to format 
        * floating-point values on trace operations.
        * Example:
        * @code{.c}
        * float myNumber = 3.5787154;
        * trace::log << trace::pre( 5 ) << myNumber << trace::endl;
        * @endcode
        */
        class pre {
            public:
                uint8_t precision;
                pre( uint8_t p ) : precision( p ) {}
        };

        /**
        * @brief Modifies the default numeric base to decimal for integer 
        * trace output
        * Example:
        * @code{.c}
        * uint16_t myNumber = 1000;
        * trace::log << trace::dec << myNumber << trace::endl;
        * @endcode
        */
        extern const tout_base dec;
        /**
        * @brief Modifies the default numeric base to hexadecimal for integer 
        * trace output
        * Example:
        * @code{.c}
        * uint16_t myNumber = 1000;
        * trace::log << trace::hex << myNumber << trace::endl;
        * @endcode
        */
        extern const tout_base hex;
        /**
        * @brief Modifies the default numeric base to octal for integer 
        * trace output
        * Example:
        * @code{.c}
        * uint16_t myNumber = 1000;
        * trace::log << trace::oct << myNumber << trace::endl;
        * @endcode
        */
        extern const tout_base oct;
        /**
        * @brief Modifies the default numeric base to binary for integer 
        * trace output
        * Example:
        * @code{.c}
        * uint16_t myNumber = 1000;
        * trace::log << trace::bin << myNumber << trace::endl;
        * @endcode
        */
        extern const tout_base bin;
        /**
        * @brief Inserts a new-line character to the trace output.
        * Example:
        * @code{.c}
        * trace::log << "hello world!" << trace::endl;
        * @endcode
        */
        extern const char * const endl;
        /**
        * @brief Inserts a new-line character to the trace output and restore
        * the default color
        * Example:
        * @code{.c}
        * trace::log << "hello world!" << trace::end;
        * @endcode
        */
        extern const char * const end;
        /**
        * @brief Set colored output to "normal" after the usage of this statement
        * Example:
        * @code{.c}
        * trace::log << trace::nrm <<"normal colored!" << trace::end;
        * @endcode
        */
        extern const char * const nrm;
        /**
        * @brief Set colored output to "red" after the usage of this statement
        * Example:
        * @code{.c}
        * trace::log << trace::red <<"red colored!" << trace::end;
        * @endcode
        */
        extern const char * const red;
        /**
        * @brief Set colored output to "green" after the usage of this statement
        * Example:
        * @code{.c}
        * trace::log << trace::grn <<"green colored!" << trace::end;
        * @endcode
        */
        extern const char * const grn;
        /**
        * @brief Set colored output to "yellow" after the usage of this statement
        * Example:
        * @code{.c}
        * trace::log << trace::yel <<"yellow colored!" << trace::end;
        * @endcode
        */
        extern const char * const yel;
        /**
        * @brief Set colored output to "blue" after the usage of this statement
        * Example:
        * @code{.c}
        * trace::log << trace::blu <<"blue colored!" << trace::end;
        * @endcode
        */
        extern const char * const blu;
        /**
        * @brief Set colored output to "magenta" after the usage of this statement
        * Example:
        * @code{.c}
        * trace::log << trace::mag <<"magenta colored!" << trace::end;
        * @endcode
        */
        extern const char * const mag;
        /**
        * @brief Set colored output to "cyan" after the usage of this statement
        * Example:
        * @code{.c}
        * trace::log << trace::cyn <<"cyan colored!" << trace::end;
        * @endcode
        */
        extern const char * const cyn;
        /**
        * @brief Set colored output to "white" after the usage of this statement
        * Example:
        * @code{.c}
        * trace::log << trace::wht <<"white colored!" << trace::end;
        * @endcode
        */
        extern const char * const wht;

        /*! @cond */
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

            #if defined( ARDUINO_PLATFORM )
                friend _trace& operator<<( _trace& tout, const String & s );
            #else
                friend _trace& operator<<( _trace& tout, const string & s );
            #endif
        };
        extern _trace& _trace_out;
        /*! @endcond */

        /**
        * @brief Set the output method for the trace stream.
        * @param[in] fcn The basic output byte function.
        * @return none.
        */
        inline void setOutputFcn( util::putChar_t fcn )
        {
            if ( nullptr != fcn ) {
                _trace_out.writeChar = fcn;
            }
        }

        /*! @cond */
        inline void log(void) {}
        inline void msg(void) {}
        inline const char * var( const char * vname ){ return vname; }
        /*! @endcond */

        /** @}*/
    }
    /** @}*/
}

/*! @cond */
#define _logHeader()                                                           \
qOS::trace::_trace_out << "[ " << qOS::trace::dec <<                           \
static_cast<unsigned long>( qOS::clock::getTick() )                            \
<< "] " << _TRACE_CURRENT_FUNCTION << ":" _TRACE_TOSTRING(__LINE__) " - "      \


#define var(v)  var( _TRACE_STRINGIFY(v) ) << '=' << v
#define log     log();_logHeader()
#define msg     msg();qOS::trace::_trace_out

/*! @endcond */

#endif /*QOS_CPP_TRACE*/
