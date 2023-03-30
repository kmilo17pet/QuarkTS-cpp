#ifndef QOS_CPP_CLI
#define QOS_CPP_CLI

#include "types.hpp"
#include "util.hpp"

namespace qOS {

    class commandLineInterface;
    namespace cli {

        enum response : std::int16_t {
            ERROR = -32767,
            NOT_ALLOWED = -32766,
            NO_RESPONSE = 0,
            OK = 1,
            DEVID = 32765,
            NOT_FOUND = 32766,
            OUTPUT = 32767,
        };

        constexpr response ERROR_CODE( int16_t code )
        {
            return static_cast<response>( -code );
        }

        enum commandType : std::uint16_t {
            UNDEF = 0x0000,
            PARA = 0x0100,
            TEST = 0x0200,
            READ = 0x0400,
            ACT = 0x0800,
        };

        class input {
            protected:
                char *storage{ nullptr };
                volatile index_t index{ 0u };
                index_t maxIndex{ 0u };
                std::size_t size{ 0u };
                volatile bool ready{ false };
                void flush( void );
                input( input const& ) = delete;      /* not copyable*/
                void operator=( input const& ) = delete;  /* not assignable*/
        };

        class _Handler {
            protected:
                commandLineInterface *instance{ nullptr };
                void *Command{ nullptr };
                char *StrData{ nullptr };
                char *Output{ nullptr };
                void *Data{ nullptr };
                std::size_t StrLen{ 0u };
                std::size_t NumArgs{ 0u };
                commandType Type{ UNDEF };
                _Handler( _Handler const& ) = delete;      /* not copyable*/
                void operator=( _Handler const& ) = delete;  /* not assignable*/
            public:
                inline char *getStringData( void ) 
                {
                    return StrData;
                }
                inline void *getData( void )
                {
                    return Data;
                }
                inline std::size_t getStringLength( void ) const
                {
                    return StrLen;
                }
                inline std::size_t getNumArgs( void ) const
                {
                    return NumArgs;
                }
                char* getArgPtr( index_t n ) const;
                int getArgInt( index_t n ) const;
                float32_t getArgFloat( index_t n ) const;
                std::uint32_t getArgHex( index_t n ) const;
                char* getArgString( index_t n, char *pOut );
                void output( const char c ) const;
                void output( const char *s ) const;
            friend class qOS::commandLineInterface;
        };
        using handler_t = _Handler&;

        using commandCallback_t = response (*)( handler_t );
        using options_t = std::uint16_t;

        class command {
            protected:
                commandCallback_t cmdCallback{ nullptr };
                command* next{ nullptr };
                options_t cmdOpt{ 0u };
                std::size_t cmdLen{ 0u };
                void *param{ nullptr };
                char *Text{ nullptr };
                command( command const& ) = delete;      /* not copyable*/
                void operator=( command const& ) = delete;  /* not assignable*/
            friend class qOS::commandLineInterface;
        };

    }

    class commandLineInterface : protected cli::input {
        protected:
            cli::_Handler handler;
            cli::command *first{ nullptr };
            const char *ok_rsp{ "OK" };
            const char *er_rsp{ "ERROR" };
            const char *nf_rsp{ "UNKNOWN" };
            const char *id_rsp{ "" };
            const char *eol{ "\r\n" };
            char delim{ ',' };
            util::putChar_t outputFcn{ nullptr };
            std::size_t sizeOutput{ 0u };
            void *owner;
            bool notify( void );
            bool preProcessing( cli::command *cmd, char *inputBuffer );
            std::size_t numOfArgs( const char *str );
            void handleResponse( cli::response retval );
            void (*xNotifyFcn)( commandLineInterface *arg) = { nullptr };
            commandLineInterface( commandLineInterface const& ) = delete;      /* not copyable*/
            void operator=( commandLineInterface const& ) = delete; /* not assignable*/
        public:
            commandLineInterface() = default;
            bool setup( util::putChar_t outFcn, char *pInput, const std::size_t sizeInput, char *pOutput, const std::size_t sizeOutput );
            bool add( cli::command &cmd, char *textCommand, cli::commandCallback_t cFcn, cli::options_t cmdOpt, void *param );
            inline bool add( cli::command &cmd, char *textCommand, cli::commandCallback_t cFcn, cli::options_t cmdOpt )
            {
                return add( cmd, textCommand, cFcn, cmdOpt, nullptr );
            }
            bool isrHandler( const char c );
            bool isrHandler( char *pData, const std::size_t n );
            bool raise( const char *cmd );
            bool inputFlush( void );
            cli::response exec( const char *cmd );
            bool run( void );
            inline void* getOwner( void )
            {
                return owner;
            }
            inline void setData( void *pData )
            {
                handler.Data = pData;
            }
        friend class cli::_Handler;
        friend class core;
    };

}



#endif /*QOS_CPP_CLI*/
