#pragma once

#include "types.hpp"

namespace qOS {
    namespace cli {

        enum commandType : std::uint16_t {
            CMDTYPE_UNDEF = 0x0000,
            CMDTYPE_PARA = 0x0100,
            CMDTYPE_TEST = 0x0200,
            CMDTYPE_READ = 0x0400,
            CMDTYPE_PARA = 0x0800,
        };

        enum response : std::int16_t {
            ERROR = -32767,
            NOT_ALLOWED = -32766,
            NO_RESPONSE = 0,
            OK = 1,
            DEVID = 32765,
            NOT_FOUND = 32766,
            OUTPUT = 32767,
        };

        class input {
            protected:
                char *storage;
                volatile index_t index;
                index_t maxIndex;
                std::size_t size;
                volatile bool ready;
                input( input const& ) = delete;      /* not copyable*/
                void operator=( input const& ) = delete;  /* not assignable*/
        };

        class _Handler {
            protected:
                void *Command{ nullptr };
                void *StrData{ nullptr };
                void *Output{ nullptr };
                void *Data{ nullptr };
                std::size_t StrLen{ 0u };
                std::size_t numArgs{ 0u };
                commandType Type{ CMDTYPE_UNDEF };
            public:
                char* getArgPtr( index_t n );
                int* getArgInt( index_t n );
                float32_t getArgFloat( index_t n );
                std::uint32_t getArgHex( index_t n );
                char *getArgString( index_t n, char *pOut );
                void putChar( const char c );
                void putString( const char c );
        };
        using handler_t = _Handler&;

        using commandCallback_t = response (*)( handler_t );
        using options_t = std::uint16_t ;
        using putChar_t = void (*)( void*, const char );

        class command {
            protected:
                commandCallback_t cmdCallback;
                command* next;
                options_t cmdOpt;
                std::size_t cmdLen;
                void *param;
                char *Text;
            public:
        };

    }

    class commandLineInterface : protected cli::input {
        protected:
            void *first{ nullptr };
            const char *ok_rsp{ "OK" };
            const char *er_rsp{ "ERROR" };
            const char *nf_rsp{ "UNKNOWN" };
            const char *id_rsp{ "QuarkTS CLI" };
            const char *eol{ "\r\n" };
            cli::putChar_t outputFcn{ nullptr };
            std::size_t sizeOutput;
            cli::input xInput;
            cli::_Handler Handler;
            void *owner;
        public:
            commandLineInterface() = default;
            bool setup( cli::putChar_t outFcn, char *pIntput, const std::size_t sizeInput, char *pOutput, const std::size_t sizeOutput );
    };
}