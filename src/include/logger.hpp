#ifndef QOS_CPP_LOGGER
#define QOS_CPP_LOGGER

#include "include/types.hpp"
#include "include/util.hpp"
#include "include/clock.hpp"
#include "include/task.hpp"
#include "include/fsm.hpp"
#include "include/timer.hpp"
#include "include/input.hpp"
#include "include/queue.hpp"
#include "include/memory.hpp"
#include "config/config.h"

namespace qOS {

    /** @addtogroup qlogger Logger
    * @brief API interfaces to print out logging messages.
    *  @{
    */

    #ifdef DOXYGEN
        /**
        * @brief The global class to output logging streams. Its usage requires
        * the static method: logger::out()
        */
        class logger final {
            public:
                /**
                * @brief Specify a new logger output with severity level of
                * information (if defined).
                * @note Should be used only at the beginning of logger stream
                * @param[in] s Severity of the message
                *
                * Example 1:
                * @code{.c}
                * logger::out() << "some message" << logger::endl;
                * @endcode
                * Example 2:
                * @code{.c}
                * logger::out(logger::info) << "some information message" << logger::endl;
                * @endcode
                * Example 3:
                * @code{.c}
                * logger::out(logger::debug) << "some debug message" << logger::endl;
                * @endcode
                */
                static logger& out( const logSeverity s );
                /**
                * @brief Specify that the variable given by @a v should be printed
                * with its own name  :  <var::name> = <var::value>
                * @param[in] v variable to be logged
                * @code{.c}
                * int myVariable;
                * logger::out() << logger::var( myVariable ) << logger::endl;
                * @endcode
                */
                static logger& var( const void &v );
        };
    #endif

    /**
    * @brief Logger interfaces.
    */
    namespace logger {

        /** @addtogroup qlogger
        *  @{
        */

       /*! @cond */
        struct source_location {
            public:
            #if not defined(__apple_build_version__) and defined(__clang__) and (__clang_major__ >= 9)
                static constexpr source_location current(const char* fileName = __builtin_FILE(),
                    const char* functionName = __builtin_FUNCTION(),
                    const unsigned long lineNumber = __builtin_LINE(),
                    const unsigned long columnOffset = __builtin_COLUMN() ) noexcept
            #elif defined(__GNUC__) and (__GNUC__ > 4 or (__GNUC__ == 4 and __GNUC_MINOR__ >= 8))
                static constexpr source_location current(const char* fileName = __builtin_FILE(),
                    const char* functionName = __builtin_FUNCTION(),
                    const unsigned long lineNumber = __builtin_LINE(),
                    const unsigned long columnOffset = 0 ) noexcept
            #else
                static constexpr source_location current(const char* fileName = "unsupported",
                    const char* functionName = "unsupported",
                    const unsigned long lineNumber = __LINE__,
                    const unsigned long columnOffset = 0) noexcept
            #endif
                {
                    return source_location( fileName, functionName, lineNumber, columnOffset ); // skipcq: CXX-W2033
                }

                source_location( const source_location & ) = default;
                source_location( source_location && ) = default;

                constexpr const char* file_name( void ) const noexcept
                {
                    return fileName;
                }

                constexpr const char* function_name( void ) const noexcept
                {
                    return functionName;
                }

                constexpr unsigned long line( void ) const noexcept
                {
                    return lineNumber;
                }

                constexpr unsigned long column( void ) const noexcept
                {
                    return columnOffset;
                }

            private:
                constexpr source_location( const char* FileName, const char* FunctionName, const unsigned long LineNumber, const unsigned long ColumnOffset ) noexcept
                    : fileName(FileName), functionName(FunctionName), lineNumber(LineNumber), columnOffset(ColumnOffset) {}

                const char* fileName;
                const char* functionName;
                const unsigned long lineNumber; // skipcq: CXX-W2010
                const unsigned long columnOffset; // skipcq: CXX-W2010
        };

        class lout_base final {
            public:
                uint8_t base;
                explicit lout_base(uint8_t b) : base(b) {}
        };
        /*! @endcond */

        enum logSeverity {
            none = 0,
            fatal = 1,
            error = 2,
            warning = 3,
            info = 4,
            debug = 5,
            verbose = 6,
        };

        /**
        * @brief Class that sets the number of bytes to be logged when a pointer
        * is being used after.
        * Example:
        * @code{.c}
        * uint32_t myNumber = 0xAABBCCDD;
        * logger::out() << logger::mem( sizeof(myNumber ) ) << &myNumber << logger::endl;
        * @endcode
        */
        class mem final {
            public:
                 /*! @cond */
                size_t n;
                /*! @endcond */
                /**
                * @brief Instantiates a memory specifier to logger @a nb bytes.
                * @param[in] nb Number of bytes to be logged.
                */
                explicit mem( size_t nb ) : n( nb ) {}
        };

        /**
        * @brief Class that sets the decimal precision to be used to format
        * floating-point values on logger operations.
        * Example:
        * @code{.c}
        * float myNumber = 3.5787154;
        * logger::out() << logger::pre( 5 ) << myNumber << logger::endl;
        * @endcode
        */
        class pre final {
            public:
                /*! @cond */
                uint8_t precision;
                /*! @endcond */
                /**
                * @brief Instantiates a precision specifier of @a p decimal points.
                * @param[in] p Number of decimal points for the precision
                */
                explicit pre( uint8_t p ) : precision( p ) {}
        };

        /**
        * @brief Modifies the default numeric base to decimal for integer
        * logger output
        * Example:
        * @code{.c}
        * uint16_t myNumber = 1000;
        * logger::out() << logger::dec << myNumber << logger::endl;
        * @endcode
        */
        extern const lout_base dec;
        /**
        * @brief Modifies the default numeric base to hexadecimal for integer
        * logger output
        * Example:
        * @code{.c}
        * uint16_t myNumber = 1000;
        * logger::out() << logger::hex << myNumber << logger::endl;
        * @endcode
        */
        extern const lout_base hex;
        /**
        * @brief Modifies the default numeric base to octal for integer
        * logger output
        * Example:
        * @code{.c}
        * uint16_t myNumber = 1000;
        * logger::out() << logger::oct << myNumber << logger::endl;
        * @endcode
        */
        extern const lout_base oct;
        /**
        * @brief Modifies the default numeric base to binary for integer
        * logger output
        * Example:
        * @code{.c}
        * uint16_t myNumber = 1000;
        * logger::out() << logger::bin << myNumber << logger::endl;
        * @endcode
        */
        extern const lout_base bin;
        /**
        * @brief Inserts a new-line character to the logger output.
        * Example:
        * @code{.c}
        * logger::out() << "hello world!" << logger::endl;
        * @endcode
        */
        extern const char * const endl;
        /**
        * @brief Inserts a new-line character to the logger output and restore
        * the default color
        * Example:
        * @code{.c}
        * logger::out() << "hello world!" << logger::end;
        * @endcode
        */
        extern const char * const end;
        /**
        * @brief Set colored output to "normal" after the usage of this statement
        * Example:
        * @code{.c}
        * logger::out() << logger::nrm <<"normal colored!" << logger::end;
        * @endcode
        */
        extern const char * const nrm;
        /**
        * @brief Set colored output to "red" after the usage of this statement
        * Example:
        * @code{.c}
        * logger::out() << logger::red <<"red colored!" << logger::end;
        * @endcode
        */
        extern const char * const red;
        /**
        * @brief Set colored output to "green" after the usage of this statement
        * Example:
        * @code{.c}
        * logger::out() << logger::grn <<"green colored!" << logger::end;
        * @endcode
        */
        extern const char * const grn;
        /**
        * @brief Set colored output to "yellow" after the usage of this statement
        * Example:
        * @code{.c}
        * logger::out() << logger::yel <<"yellow colored!" << logger::end;
        * @endcode
        */
        extern const char * const yel;
        /**
        * @brief Set colored output to "blue" after the usage of this statement
        * Example:
        * @code{.c}
        * logger::out() << logger::blu <<"blue colored!" << logger::end;
        * @endcode
        */
        extern const char * const blu;
        /**
        * @brief Set colored output to "magenta" after the usage of this statement
        * Example:
        * @code{.c}
        * logger::out() << logger::mag <<"magenta colored!" << logger::end;
        * @endcode
        */
        extern const char * const mag;
        /**
        * @brief Set colored output to "cyan" after the usage of this statement
        * Example:
        * @code{.c}
        * logger::out() << logger::cyn <<"cyan colored!" << logger::end;
        * @endcode
        */
        extern const char * const cyn;
        /**
        * @brief Set colored output to "white" after the usage of this statement
        * Example:
        * @code{.c}
        * logger::out() << logger::wht <<"white colored!" << logger::end;
        * @endcode
        */
        extern const char * const wht;

        struct ChainLoggerProxy;

        /*! @cond */
        class _logger final : private nonCopyable {
            private:
                _logger() = default;
                _logger( _logger &other ) = delete;
                void operator=( const _logger & ) = delete;
                const char *s_str[ 7 ] = { "", "[fatal]: ", "[error]: ", "[warning]: ", "[info]: ", "[debug] ", "" }; // skipcq: CXX-W2066
                uint8_t base = { 10U };
                size_t n{ 0U };
                uint8_t precision { 6U };
                #if ( Q_TRACE_BUFSIZE < 36 )
                    #define Q_TRACE_BUFSIZE     ( 36 )
                #endif
                char buffer[ Q_TRACE_BUFSIZE ] = { 0 }; // skipcq: CXX-W2066
                char preFix[ 5 ] = { 0 }; // skipcq: CXX-W2066
                util::putChar_t writeChar{ nullptr };
                void writeNumStr( void ) noexcept;
            public:
                static _logger& getInstance( void ) noexcept;

                template <typename T>
                auto toLog(const T& v) -> decltype((void)(T(1) % 1), (void)(-T(1)), void()) // only valid for integrals
                {
                    /*cstat -CERT-INT31-C_a -MISRAC++2008-5-0-8 -MISRAC++2008-5-0-9 -MISRAC++2008-0-1-2_b -MISRAC++2008-0-1-2_a*/
                    if (T(-1) < T(0)) {
                        (void)util::integerToString(static_cast<signed_t>(v), buffer, base); // skipcq: CXX-C1000
                    } else {
                        (void)util::unsignedToString(static_cast<unsigned_t>(v), buffer, base); // skipcq: CXX-C1000
                    }
                    /*cstat +CERT-INT31-C_a +MISRAC++2008-5-0-8 +MISRAC++2008-5-0-9 +MISRAC++2008-0-1-2_b +MISRAC++2008-0-1-2_a*/
                    writeNumStr();
                }

                void toLog( const char& v );
                void toLog( const char * s );
                void toLog( const void * const p );

                void toLog( const float32_t& v );
                void toLog( const float64_t& v );
                void toLog( const lout_base& f );
                void toLog( const mem& m );
                void toLog( const pre& m );
                void toLog( const qOS::task& t );
                void toLog( const qOS::timer& t );
                void toLog( const qOS::stateMachine& sm );
                void toLog( const qOS::sm::state& s );
                void toLog( const qOS::trigger& t );
                void toLog( const qOS::globalState& s );
                void toLog( const qOS::taskEvent& e );
                void toLog( const qOS::taskState& v );
                void toLog( const qOS::queue& v );
                void toLog( const qOS::mem::pool& v );
                void toLog( const qOS::input::channel& in );
                void toLog( const qOS::input::watcher& v );
                void toLog( const qOS::sm::signalID& v );
                void toLog( const qOS::sm::status& v );
                void toLog( const qOS::sm::stateHandler& v );
                void toLog( const qOS::string & s );

            friend ChainLoggerProxy out( const logSeverity s, const source_location &loc ) noexcept;
            friend void setOutputFcn( util::putChar_t fcn );
            friend class ChainLoggerProxy;
        };
        extern _logger& _logger_out; // skipcq: CXX-W2011, CXX-W2009

        #define IMPL_CHAIN_LOGGER_OPERATOR( Type )                          \
        ChainLoggerProxy& operator<<( Type v )                              \
        {                                                                   \
            parent.toLog( v );                                              \
            return *this;                                                   \
        }                                                                   \

        struct ChainLoggerProxy {
            _logger& parent; // skipcq: CXX-W2010
            explicit ChainLoggerProxy( _logger& p) : parent(p) {}
            ChainLoggerProxy( const ChainLoggerProxy& ) = delete;
            ChainLoggerProxy( ChainLoggerProxy&& other ) noexcept : parent( other.parent ) {}
            ~ChainLoggerProxy();

            template<typename T>
            auto operator<<(const T& v) -> decltype((void)(v + 0), *this)
            {
                parent.toLog( v );
                return *this;
            }
            IMPL_CHAIN_LOGGER_OPERATOR( const char& )
            IMPL_CHAIN_LOGGER_OPERATOR( const char * )
            IMPL_CHAIN_LOGGER_OPERATOR( const void * const )
            IMPL_CHAIN_LOGGER_OPERATOR( const lout_base& )
            IMPL_CHAIN_LOGGER_OPERATOR( const mem& )
            IMPL_CHAIN_LOGGER_OPERATOR( const pre& )
            IMPL_CHAIN_LOGGER_OPERATOR( const qOS::task& )
            IMPL_CHAIN_LOGGER_OPERATOR( const qOS::timer& )
            IMPL_CHAIN_LOGGER_OPERATOR( const qOS::stateMachine& )
            IMPL_CHAIN_LOGGER_OPERATOR( const qOS::sm::state& )
            IMPL_CHAIN_LOGGER_OPERATOR( const qOS::trigger& )
            IMPL_CHAIN_LOGGER_OPERATOR( const qOS::globalState& )
            IMPL_CHAIN_LOGGER_OPERATOR( const qOS::taskEvent& )
            IMPL_CHAIN_LOGGER_OPERATOR( const qOS::taskState& )
            IMPL_CHAIN_LOGGER_OPERATOR( const qOS::queue& )
            IMPL_CHAIN_LOGGER_OPERATOR( const qOS::mem::pool& )
            IMPL_CHAIN_LOGGER_OPERATOR( const qOS::input::channel& )
            IMPL_CHAIN_LOGGER_OPERATOR( const qOS::input::watcher& )
            IMPL_CHAIN_LOGGER_OPERATOR( const qOS::sm::signalID& )
            IMPL_CHAIN_LOGGER_OPERATOR( const qOS::sm::status& )
            IMPL_CHAIN_LOGGER_OPERATOR( const qOS::sm::stateHandler& )
            IMPL_CHAIN_LOGGER_OPERATOR( const qOS::string& )
        };
        /*! @endcond */

        /**
        * @brief Set the output method for the logger stream.
        * @param[in] fcn The basic output byte function.
        */
        void setOutputFcn( util::putChar_t fcn );

        /*! @cond */
        ChainLoggerProxy out( const logSeverity s = logSeverity::none, const source_location &loc = source_location::current() ) noexcept;
        /* cppcheck-suppress functionStatic */
        inline const char * var( const char * vname ){ return vname; }
        /*! @endcond */

        /** @}*/
    }
    /** @}*/
}

/*! @cond */
#define var(v)  var( #v ) << '=' << (v)
/*! @endcond */

#endif /*QOS_CPP_LOGGER*/
