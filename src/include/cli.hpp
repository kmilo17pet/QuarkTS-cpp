#ifndef QOS_CPP_CLI
#define QOS_CPP_CLI

#include "include/types.hpp"
#include "include/list.hpp"
#include "include/util.hpp"

namespace qOS {

    class commandLineInterface;

    /** @addtogroup  qatcli AT Command Line Interface
    * @brief API for the @ref qatcli extension.
    *  @{
    */

    /**
    * @brief AT command line interfaces.
    */
    namespace cli {

        /** @addtogroup  qatcli
        *  @{
        */

        /**
        * @brief an enumeration to define the possible values that can be returned
        * from the callback of a command.
        */
        enum response : int16_t {
            ERROR = -32767,
            NOT_ALLOWED = -32766,
            NO_RESPONSE = 0,
            OK = 1,
            DEVID = 32765,
            NOT_FOUND = 32766,
            OUTPUT_RESPONSE = 32767,
        };

        /**
        * @brief Used to indicate an error code as return value inside a
        * command-callback.
        * This code is defined by the application writer and should be a value
        * between @c 1 and @c 32766.
        *
        * For example, a return value of @c cli::ERROR_CODE(15), will print out the
        * string @c ERROR:15.
        */
        constexpr response ERROR_CODE( int16_t code )
        {
            return static_cast<response>( -code );
        }

        /**
        * @brief An enum to describe the available AT command types.
        */
        enum commandType : uint16_t {
            UNDEF = 0x0000, /**< Was not able to detected a correct input command*/
            PARA = 0x0100,  /**< Command that receives parameters (comma separated arguments after the equal(=) symbol) : @c "AT+cmd=x,y" */
            TEST = 0x0200,  /**< Command in test mode (no arguments allowed): @c "AT+cmd=?" */
            READ = 0x0400,  /**< Command to query information(data allowed after the ? symbol)  : @c "AT+cmd?" */
            ACT = 0x0800,   /**< Command to perform an action (no arguments allowed) : @c AT+cmd */
        };

        /*! @cond */
        class input {
            public:
                virtual ~input() {}
            protected:
                char *storage{ nullptr };
                volatile index_t index{ 0U };
                index_t maxIndex{ 0U };
                size_t size{ 0U };
                volatile bool ready{ false };
                void flush( void );
                void operator=( input const& ) = delete;
                input() {}
        };
        /*! @endcond */


        #ifdef DOXYGEN
        /**
        * @brief The command argument with all the regarding information of the
        * incoming AT command.
        * @details From the callback context, can be used to print out extra
        * information as a command response, parse the command parameters, and
        * query properties with crucial information about the
        * detected command, like the type, the number of arguments, and the
        * subsequent string after the command text.
        * @note Should be used only in command-callbacks as the only input argument.
        */
        class handler_t final {
            private:
                commandLineInterface *instance{ nullptr };
                void *Command{ nullptr };
                char *StrData{ nullptr };
                void *Data{ nullptr };
                size_t StrLen{ 0U };
                size_t NumArgs{ 0U };
                commandType Type{ UNDEF };
                _Handler( _Handler const& ) = delete;
                void operator=( _Handler const& ) = delete;
                _Handler() = default;
            public:
                /**
                * @brief Retrieve the he incoming command type.*
                * @return The command type.
                */
                inline commandType getType( void ) const
                {
                    return Type;
                }
                /**
                * @brief  The string data received after the detected command.
                * @return The string data
                */
                inline char* getStringData( void )
                {
                    return StrData;
                }
                /**
                * @brief Retrieve a pointer to the user-defined data - Storage Pointer.
                * @return A pointer to the user-defined data.
                */
                inline void* getData( void )
                {
                    return Data;
                }
                /**
                * @brief The length of the string@a StrData.
                */
                inline size_t getStringLength( void ) const
                {
                    return StrLen;
                }
                /**
                * @brief Retrieve the number of arguments of the incoming AT command.
                * only available if Type = commandType::PARA
                * @return The number of arguments of the incoming AT command
                */
                inline size_t getNumArgs( void ) const
                {
                    return NumArgs;
                }
                /**
                * @brief Helper method to get the pointer where the desired argument
                * starts.
                * @param[in] n The number of the argument
                * @return A pointer to the desired argument. @c NULL pointer if the
                * argument is not present.
                */
                char* getArgPtr( index_t n ) const;
                /**
                * @brief Helper method to get the @a n argument parsed as integer from
                * the incoming AT command.
                * @see util::stringToInteger()
                * @param[in] n The number of the argument
                * @return The argument parsed as integer. Same behavior of
                * util::stringToInteger(). If argument not found returns 0.
                */
                int getArgInt( index_t n ) const;
                /**
                * @brief Helper method to get the @a n argument parsed as float from
                * the incoming AT command.
                * @see util::stringToFloat()
                * @param[in] n The number of the argument
                * @return The argument parsed as Float. Same behavior of util::stringToFloat().
                * If argument not found returns 0.0f
                */
                float32_t getArgFloat( index_t n ) const;
                /**
                * @brief Helper method to get the @a n HEX argument parsed @c uint32_t
                * from the incoming AT command.
                * @see util::hexStringToUnsigned()
                * @param[in] n The number of the argument
                * @return The HEX argument parsed as @c uint32_t. Same behavior of
                * util::hexStringToUnsigned()(). If argument not found returns 0.
                */
                uint32_t getArgHex( index_t n ) const;
                /**
                * @brief Helper method to get the @a n argument parsed as string from
                * the incoming AT command.
                * @param[in] n The number of the argument
                * @param[out] pOut Array in memory where to store the resulting
                * null-terminated string.
                * @return Same as @a out on success, otherwise returns @c nullptr.
                */
                char* getArgString( index_t n, char *pOut );
                /**
                * @brief Helper method for printing a character to the CLI output.
                * It displays only one character at a time.
                * @param[in] c The ASCII character.
                * @return none.
                */
                void writeOut( const char c ) const;
                /**
                * @brief Writes a string to CLI output without the @c EOF string appended
                * at the end.
                * @param[in] s This is the C string to be written.
                * @return none.
                */
                void writeOut( const char *s ) const;
                /**
                * @brief The CLI output buffer. Can be written by the user.
                */
                char *output{ nullptr };
                /**
                * @brief return the instance of command being evaluated
                * @return The command instance.
                */
                inline command& thisCommand( void ) noexcept;
            friend class qOS::commandLineInterface;
        };
        #endif

        class command;
        /*! @cond  */
        class commandHandler final {
            private:
                commandLineInterface *instance{ nullptr };
                command *Command{ nullptr };
                char *StrData{ nullptr };
                void *Data{ nullptr };
                size_t StrLen{ 0U };
                size_t NumArgs{ 0U };
                commandType Type{ UNDEF };
                commandHandler( commandHandler const& ) = delete;
                void operator=( commandHandler const& ) = delete;
                commandHandler() = default;
            public:
                inline commandType getType( void ) const
                {
                    return Type;
                }
                inline char* getStringData( void )
                {
                    return StrData;
                }
                inline void* getData( void )
                {
                    return Data;
                }
                inline size_t getStringLength( void ) const
                {
                    return StrLen;
                }
                inline size_t getNumArgs( void ) const
                {
                    return NumArgs;
                }
                inline command& self( void ) noexcept
                {
                    return *Command;
                }
                inline command& thisCommand( void ) noexcept
                {
                    return *Command;
                }
                char* getArgPtr( index_t n ) const;
                int getArgInt( index_t n ) const;
                float32_t getArgFloat( index_t n ) const;
                uint32_t getArgHex( index_t n ) const;
                char* getArgString( index_t n, char *pOut );
                void writeOut( const char c ) const;
                void writeOut( const char *s ) const;
                char *output{ nullptr };
            friend class qOS::commandLineInterface;
        };
        using handler_t = commandHandler&;
        /*! @endcond  */

        /**
        * @brief Pointer to function : An AT-Command callback
        *
        * Example :
        * @code{.c}
        * cli::response commandCallback_example( cli::handler_t h ) {
        *       cli::response retValue = cli::response::ERROR;
        *       switch ( h.getType() ) {
        *           case commandType::ACT :
        *               h.output( "Test message" );
        *               retValue = cli::response::OK;
        *               break;
        *           default:
        *               break;
        *       }
        *       return retValue;
        * }
        * @endcode
        * @param h A reference to the CLI command handler instance
        * @return The command response.
        */
        using commandCallback_t = response (*)( handler_t );

        /**
        * @brief A typedef that holds the options for an AT-Command object
        */
        using options_t = uint16_t;

        /**
        * @brief An AT-Command object
        */
        class command : protected node {
            private:
                commandCallback_t cmdCallback{ nullptr };
                options_t cmdOpt{ 0U };
                size_t cmdLen{ 0U };
                void *param{ nullptr };
                char *Text{ nullptr };
                command( command const& ) = delete;
                void operator=( command const& ) = delete;
            public:
                command() = default;
                virtual ~command() {}
                inline void* getParam( void ) noexcept
                {
                    return param;
                }
            friend class qOS::commandLineInterface;
        };
        /** @}*/
    }

    /**
    * @brief An AT Command Line Interface (CLI) object
    * @details Before starting the CLI development, the corresponding instance
    * must be defined.
    *
    * The instance should be initialized using the commandLineInterface::setup()
    * method.
    */
    class commandLineInterface : protected cli::input {
        private:
            list subscribed;
            cli::commandHandler handler;
            const char *ok_rsp{ "OK" };
            const char *er_rsp{ "ERROR" };
            const char *nf_rsp{ "UNKNOWN" };
            const char *id_rsp{ "" };
            const char *eol{ "\r\n" };
            char delim{ ',' };
            util::putChar_t outputFcn{ nullptr };
            size_t sizeOutput{ 0U };
            void *owner{ nullptr };
            bool notify( void );
            bool preProcessing( cli::command *cmd, char *inputBuffer );
            size_t numOfArgs( const char *str ) const;
            void handleResponse( cli::response retval );
            void (*xNotifyFcn)( commandLineInterface *arg) = { nullptr };
            commandLineInterface( commandLineInterface const& ) = delete;
            void operator=( commandLineInterface const& ) = delete;
        public:
            commandLineInterface() = default;
            virtual ~commandLineInterface() {}
            /**
            * @brief Setup an instance of the AT Command Line Interface.
            * @see core::addCommandLineInterfaceTask()
            * @note CLI Built-in strings will be written to their default values.
            * @param[in] outFcn The basic output-char wrapper function. All the CLI
            * responses will be printed-out through this function.
            * @param[in] pInput A memory location to store the cli input (Mandatory)
            * @param[in] sizeIn The size of the memory allocated in @a pInput
            * @param[in] pOutput A memory location to store the CLI output
            * @param[in] sizeOut The size of the memory allocated in @a pOutput
            * @return @c true on success, otherwise return @c false.
            */
            bool setup( util::putChar_t outFcn, char *pInput, const size_t sizeIn, char *pOutput, const size_t sizeOut );
            template <size_t inBufferSize, size_t outBufferSize>
            bool setup( util::putChar_t outFcn, char (&pInput)[ inBufferSize ], char (&pOutput)[ outBufferSize ] ) // skipcq : CXX-W2066
            {
                return setup( outFcn, pInput, inBufferSize, pOutput, outBufferSize );
            }
            /**
            * @brief This function subscribes the CLI instance to a specific command
            * with an associated @a Callback function, so that next time the required
            * command is sent to the CLI input, the callback function will be executed.
            * The CLI parser only analyze commands that follows the extended AT-Commands
            * syntax (the + char can be ignored).
            * @param[in] cmd The AT command object.
            * @param[in] textCommand The string (name) of the command we want to
            * subscribe to. Since this service only handles AT commands, this string has
            * to begin by the "at" characters and should be in lower case.
            * @param[in] cFcn The handler of the callback function associated to the
            * command.
            * Prototype: @code cli::response xCallback( cli::handler_t h ) @endcode
            * @param[in] cmdOpt This flag combines with a bitwise OR the following
            * information:
            *
            * cli::commandType::PARA  : @c AT+cmd=x,y is allowed. The execution of the
            * callback function also depends on whether the number of argument is valid
            * or not. Information about number of arguments is combined with a bitwise
            * 'OR' : cli::commandType::PARA | 0xXY , where X which defines maximum
            * argument number for incoming command and Y which defines minimum argument
            * number for incoming command
            *
            * cli::commandType::TEST  : @c "AT+cmd=?" is allowed.
            *
            * cli::commandType::READ  : @c "AT+cmd?" is allowed.
            *
            * cli::commandType::ACT   : @c AT+cmd is allowed.
            *
            * @param[in] param User storage pointer.
            * @return @c true on success, otherwise return @c false.
            */
            bool add( cli::command &cmd, char *textCommand, const cli::commandCallback_t &cFcn, cli::options_t cmdOpt, void *param = nullptr );
            /**
            * @brief Feed the CLI input with a single character. This call is mandatory
            * from an interrupt context. Put it inside the desired peripheral's ISR.
            * @param[in] c The incoming byte/char to the input.
            * @return @c true when the CLI is ready to process the input, otherwise
            * return @c false
            */
            bool isrHandler( const char c );
            /**
            * @brief Feed the CLI input with a string. This call is mandatory
            * from an interrupt context. Put it inside the desired peripheral's ISR.
            * @param[in] pData The incoming string.
            * @param[in] n The length of the string.
            * @return @c true when the CLI is ready to process the input, otherwise
            * return @c false
            */
            bool isrHandler( char *pData, const size_t n );
            /**
            * @brief Sends a command to the Command Line Interface instance.
            * @param[in] cmd The command string, including arguments if required.
            * @return @c true when the CLI accepts the input. If busy, return @c false
            */
            bool raise( const char *cmd );
            /**
            * @brief Flush the CLI input buffer.
            * @return @c true on success, otherwise return @c false
            */
            bool inputFlush( void );
            /**
            * @brief Try to execute the requested command.
            * @param[in] cmd The command string, including arguments if required.
            * @return The response output for the requested command.
            */
            cli::response exec( const char *cmd );
            /**
            * @brief Run the AT Command Line Interface when the input is ready.
            * @see core::addCommandLineInterfaceTask()
            * @return @c true on success, otherwise return @c false
            */
            bool run( void );
            /**
            * @brief Retrieves a pointer of the owner of this object.
            * @return @c A pointer to the owner.
            */
            inline void* getOwner( void )
            {
                return owner;
            }
            /**
            * @brief Set the cli data or storage-pointer.
            * @param[in] pData A pointer to data or storage-pointer.
            */
            inline void setData( void *pData )
            {
                handler.Data = pData;
            }
        friend class cli::commandHandler;
        friend class core;
    };
    /** @}*/
}


#endif /*QOS_CPP_CLI*/
