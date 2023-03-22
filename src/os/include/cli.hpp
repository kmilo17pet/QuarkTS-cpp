#pragma once

#include "types.hpp"

namespace qOS {
    namespace cli {

        typedef enum {
            CMDTYPE_UNDEF = 0x0000,
            CMDTYPE_PARA = 0x0100,
            CMDTYPE_TEST = 0x0200,
            CMDTYPE_READ = 0x0400,
            CMDTYPE_PARA = 0x0800,
        } commandType;

        typedef enum {
            ERROR = -32767,
            NOT_ALLOWED = -32766,
            NO_RESPONSE = 0,
            OK = 1,
            DEVID = 32765,
            NOT_FOUND = 32766,
            OUTPUT = 32767,
        } response;

        class input {
            protected:
                char *storage;
                volatile index_t index;
                index_t maxIndex;
                size_t size;
                volatile bool ready;
        };

        class _Handler {
            protected:
                void *Command{ nullptr };
                void *StrData{ nullptr };
                void *Output{ nullptr };
                void *Data{ nullptr };
                size_t StrLen{ 0u };
                size_t numArgs{ 0u };
                commandType Type{ CMDTYPE_UNDEF };
            public:
                char* getArgPtr( index_t n );
                int* getArgInt( index_t n );
                float32_t getArgFloat( index_t n );
                uint32_t getArgHex( index_t n );
                char *getArgString( index_t n, char *pOut );
                void putChar( const char c );
                void putString( const char c );
        };
        typedef _Handler& handler_t;

        typedef response (*commandCallback_t)( handler_t h );
        typedef uint16_t options_t;
        typedef void (*putChar_t)( void* arg1, const char arg2 );

        class command {
            protected:
                commandCallback_t cmdCallback;
                command* next;
                options_t cmdOpt;
                size_t cmdLen;
                void *param;
                char *Text;
            public:
        };

    }



    class commandLineInterface {
        protected:
            void *first{ nullptr };
            const char *ok_rsp{ "OK" };
            const char *er_rsp{ "ERROR" };
            const char *nf_rsp{ "UNKNOWN" };
            const char *id_rsp{ "QuarkTS CLI" };
            const char *eol{ "\r\n" };
            cli::putChar_t outputFcn{ nullptr };
            size_t sizeOutput;
            cli::input xInput;
            cli::_Handler Handler;
            void *owner;
        public:
            commandLineInterface() = default;
            bool setup( cli::putChar_t outFcn, char *pIntput, const size_t sizeInput, char *pOutput, const size_t sizeOutput );
    };
}