#include "cli.hpp"
#include <cstring>
#include <cctype>

using namespace qOS;

static const char *Q_CLI_DEFAULT_AT_COMMAND = "at";
static const char *Q_CLI_DEFAULT_ID_COMMAND = "atid";
static const char *Q_CLI_DEFAULT_NOTALLOWED_RSP_STRING = ":NOT ALLOWED";
static const char *Q_CLI_DEFAULT_DEVID_STRING = "QuarkTS CLI";
static const size_t Q_CLI_MIN_INPUT_LENGTH = 3u;
static const size_t Q_CLI_RECOMMENDED_INPUT_SIZE = 128u;

static std::size_t CMD_MASK_ARG_MAX_NUM( cli::options_t opt );
static std::size_t CMD_MASK_ARG_MIN_NUM( cli::options_t opt );

/*============================================================================*/
static std::size_t CMD_MASK_ARG_MAX_NUM( cli::options_t opt )
{
    return ( static_cast<std::size_t>(opt) >> 4u ) & static_cast<std::size_t>( 0x000Fu );
}
/*============================================================================*/
static std::size_t CMD_MASK_ARG_MIN_NUM( cli::options_t opt )
{
    return static_cast<std::size_t>(opt)  & static_cast<std::size_t>( 0x000Fu );
}
/*============================================================================*/
static char* inputFix( char *s, std::size_t maxlen )
{
    int i, j = 0;
    int noL = 0;

    for ( i = 0 ; ( (char)'\0' != s[ i ] ) && ( maxlen > 0u ) ; ++i ) {
        if ( ( '=' == s[ i ] ) || ( '?' == s[ i ] ) ) {
            noL = 1;
        }
        if ( '\r' == s[ i ] ) {
            s[ i ] = (char)'\0';
            break;
        }
        if ( 0 != isgraph( (int)s[ i ] ) ) {
            if ( 0 == noL ) {
                s[ j++ ] = (char)tolower( (int)s[ i ] );
            }
            else {
                s[ j++ ] = s[ i ];
            }
        }
        --maxlen;
    }
    s[ j ] = (char)'\0';

    return s;
}
/*============================================================================*/
void cli::input::flush( void )
{
    (void)memset( storage, 0, size );
}
/*============================================================================*/
bool commandLineInterface::setup( util::putChar_t outFcn, char *pInput, const size_t sizeInput, char *pOutput, const size_t sizeOutput )
{
    bool retValue = false;

    if ( nullptr != outFcn ) {
        outputFcn = outFcn;
        this->sizeOutput = sizeOutput;
        handler.Output = pOutput;
        handler.instance = this;
        cli::input::storage = pInput;
        cli::input::size = sizeInput;
        cli::input::maxIndex = sizeInput - 1u;
        (void)memset( handler.Output, 0, this->sizeOutput );
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
bool commandLineInterface::add( cli::command &cmd, char *textCommand, cli::commandCallback_t cFcn, cli::options_t cmdOpt, void *param )
{
    bool retValue = false;

    if ( ( nullptr != textCommand ) && ( nullptr != cFcn ) ) {
        cmd.cmdLen = util::strlen( textCommand, cli::input::size );
        if ( cmd.cmdLen >= 2u ) {
            /*command should start with an <at> at the beginning */
            if ( ( 'a' == textCommand[ 0 ] ) && ( 't' == textCommand[ 1 ] ) ) {
                cli::command  *iCmd = nullptr;
                cmd.Text = textCommand;
                cmd.cmdCallback = cFcn; /*install the callback*/
                cmd.cmdOpt = 0x0FFFu & cmdOpt; /*high nibble not used yet*/
                cmd.param = param;
                if ( nullptr != first ) { /*list already has items*/
                    for ( iCmd = first ; nullptr != iCmd ; iCmd = iCmd->next ) {
                        if ( &cmd == iCmd ) {
                            break;
                        }
                    }
                }
                if ( &cmd != iCmd ) {
                    cmd.next = first;
                    /*command inserted at the beginning of the list*/
                    first = &cmd;
                    retValue = true;
                }
            }
        }
    }

    return retValue;
}
/*============================================================================*/
bool commandLineInterface::notify( void )
{
    cli::input::ready = true;
    cli::input::index = 0u;
    if ( nullptr != xNotifyFcn ) {
        /*external task notification if available*/
        xNotifyFcn( this );
    }

    return true;
}
/*============================================================================*/
bool commandLineInterface::isrHandler( const char c )
{
    bool retValue = false;
    bool readyInput = cli::input::ready;
    /*isgraph is known to have no-side effects*/
    /*check if the input is available and incoming chars are valid*/
    if ( ( false == readyInput ) && ( 0 != isgraph( (int)c ) ) ) {
        /*to avoid undefined order of volatile accesses*/
        index_t currentIndex = cli::input::index;
        cli::input::storage[ currentIndex++ ] = c; /*insert char*/
        /*put the null-char after to keep the string safe*/
        cli::input::storage[ currentIndex ] = (char)'\0';
        /*check if the input buffer its reached*/
        if ( currentIndex >= cli::input::maxIndex ) {
            currentIndex = 0u;
        }
        cli::input::index = currentIndex;
    }
    if ( '\r' == c ) {
        /*end of line received, send the notification to the cli*/
        retValue = notify();
    }

    return retValue;
}
/*============================================================================*/
bool commandLineInterface::isrHandler( char *pData, const std::size_t n )
{
    bool retValue = false;
    bool readyInput = cli::input::ready;
    const std::size_t maxToInsert = cli::input::maxIndex;

    if ( ( n > 0u ) && ( n < maxToInsert ) &&  ( false == readyInput ) ) {
        if ( 1u == n ) {
            retValue = isrHandler( pData[ 0 ] );
        }
        else {
            if ( 0 != isgraph( (int)pData[ 0 ] ) ) {
                /*find the end of line safely*/
                if ( nullptr != strchr( pData, (int)'\r' ) ) {
                    (void)util::strcpy( cli::input::storage, pData, maxToInsert );
                    retValue = notify();
                }
            }
        }
    }

    return retValue;
}
/*============================================================================*/
std::size_t commandLineInterface::numOfArgs( const char *str )
{
    std::size_t count = 0u;

    while ( '\0' != *str ) {
        if ( delim == (char)*str++ ) {
            ++count;
        }
    }

    return ( count + 1u );
}
/*============================================================================*/
bool commandLineInterface::preProcessing( cli::command *cmd, char *inputBuffer )
{
    bool retValue = false;

    handler.Type = cli::commandType::UNDEF;
    handler.Command = cmd;
    handler.StrLen = util::strlen( (const char*)inputBuffer, cmd->cmdLen );
    handler.StrData = (char*)&inputBuffer[ cmd->cmdLen ];
    handler.NumArgs = 0u;

    if ( 0u == handler.StrLen ) { /*command should be an ACT command */
        if ( 0u != ( cmd->cmdOpt & static_cast<cli::options_t>( cli::commandType::ACT ) ) ) {
            handler.Type = cli::commandType::ACT;  /*set the type to ACT*/
            retValue = true;
        }
    }
    else {
        if ( (char)'?' == handler.StrData[ 0 ] ) { /*command should be READ command */
            if ( 0u != ( cmd->cmdOpt & static_cast<cli::options_t>( cli::commandType::READ ) ) ) {
                handler.Type = cli::commandType::READ; /*set the type to READ*/
                ++handler.StrData; /*move string pointer once*/
                --handler.StrLen;  /*decrease the len one*/
                retValue = true;
            }
        }
        else if ( handler.StrLen >= 2u ) { /*can be at+xx=? or at+xx=...*/
            if ( (char)'=' == handler.StrData[ 0 ] ) { /*could be a TEST or PARA command*/
                if ( (char)'?' == handler.StrData[ 1 ] ) {
                    if ( ( 2u == handler.StrLen ) && ( 0u != ( cmd->cmdOpt & static_cast<cli::options_t>( cli::commandType::TEST ) ) ) ) {
                        /*command should be a TEST Command*/
                        handler.Type = cli::commandType::TEST; /*set the type to TEST*/
                        handler.StrData += 2; /*move string two positions ahead*/
                        handler.StrLen -= 2u;  /*decrease the len*/
                        retValue = true;
                    }
                }
                else { /*definitely is a PARA command*/
                    if ( 0u != ( cmd->cmdOpt & static_cast<cli::options_t>( cli::commandType::PARA ) ) ) { /*check if is allowed*/
                        std::size_t argMin = CMD_MASK_ARG_MIN_NUM( cmd->cmdOpt );
                        std::size_t argMax = CMD_MASK_ARG_MAX_NUM( cmd->cmdOpt );
                        /*get the args count using the default delimiter*/
                        handler.NumArgs = numOfArgs( handler.StrData );
                        if ( ( handler.NumArgs >= argMin ) && ( handler.NumArgs <= argMax ) ) {
                            handler.Type = cli::commandType::PARA; /*set the type to PARA*/
                            ++handler.StrData; /*move string pointer once*/
                            --handler.StrLen; /*decrease the len one*/
                            retValue = true;
                        }
                    }
                }
            }
        }
        else {
          /*nothing to do*/
        }
    }

    return retValue;
}
/*============================================================================*/
bool commandLineInterface::raise( const char *cmd )
{
    bool retValue = false;

    if ( nullptr != cmd ) {
        bool readyInput = cli::input::ready;
        std::size_t maxToInsert = cli::input::maxIndex;
        /*qIOUtil_StrLen is known to have no side effects*/
        if ( ( false == readyInput ) && ( strlen( cmd ) <= maxToInsert ) ) {
            (void)util::strcpy( (char*)cli::input::storage, cmd, maxToInsert );
            (void)inputFix( (char*)cli::input::storage, maxToInsert );
            retValue = notify();
        }
    }

    return retValue;
}
/*============================================================================*/
cli::response commandLineInterface::exec( const char *cmd )
{
    cli::response retValue = cli::response::NOT_FOUND;

    if ( nullptr != cmd ) {
        /*loop over the subscribed commands*/
        for ( cli::command *iCmd = first ; nullptr != iCmd ; iCmd = iCmd->next ) {
            /*check if the input matches the subscribed command */
            if ( 0 == strncmp( cmd, iCmd->Text, iCmd->cmdLen ) ) {
                retValue = cli::response::NOT_ALLOWED;
                if ( true == preProcessing( iCmd, (char*)cmd ) ) {
                    /*if success, proceed with the user pos-processing*/
                    cli::commandCallback_t cmdCallback = iCmd->cmdCallback;

                    if ( cli::commandType::UNDEF == handler.Type ) {
                        retValue = cli::response::ERROR;
                    }
                    else {
                        /*invoke the callback*/
                        retValue = cmdCallback( handler );
                    }
                }
                break;
            }
        }
    }

    return retValue;
}
/*============================================================================*/
bool commandLineInterface::inputFlush( void )
{
    cli::input::ready = false;
    cli::input::index = 0u;
    cli::input::storage[ 0 ] = (char)'\0';

    return true;
}
/*============================================================================*/
void commandLineInterface::handleResponse( cli::response retval )
{
    if ( cli::response::NO_RESPONSE != retval ) {

        switch ( retval ) { /*handle the command-callback response*/
            case cli::response::ERROR:
                (void)util::outputString( outputFcn, nullptr, er_rsp, false );
                break;
            case cli::response::OK:
                (void)util::outputString( outputFcn, nullptr, ok_rsp, false );
                break;
            case cli::response::NOT_ALLOWED:
                (void)util::outputString( outputFcn, nullptr, er_rsp, false );
                (void)util::outputString( outputFcn, nullptr, Q_CLI_DEFAULT_NOTALLOWED_RSP_STRING, false );
                break;
            case cli::response::DEVID:
                (void)util::outputString( outputFcn, nullptr, id_rsp, false );
                break;
            case cli::response::NOT_FOUND:
                (void)util::outputString( outputFcn, nullptr, nf_rsp, false );
                break;
            case cli::response::OUTPUT:
                (void)util::outputString( outputFcn, nullptr, handler.Output, false );
                break;
            default: /*AT_ERROR_CODE(#) */
                if ( (base_t)retval < 0 ) {
                    std::int32_t errorCode = cli::ERROR_CODE( (int32_t)retval );

                    (void)util::itoa( errorCode, handler.Output, 10u );
                    (void)util::outputString( outputFcn, nullptr, er_rsp, false );
                    outputFcn( nullptr, ':' );
                    (void)util::outputString( outputFcn, nullptr, handler.Output, false );
                    handler.Output[ 0 ] = (char)'\0';
                }
                break;
        }
        (void)util::outputString( outputFcn, nullptr, eol , false );
    }
}
/*============================================================================*/
bool commandLineInterface::run( void )
{
    bool retValue = false;

    if ( true == cli::input::ready ) { /*a new input has arrived*/
        cli::response outRetval, cliRetVal;
        char *inputBuffer = cli::input::storage;
        inputBuffer[ cli::input::maxIndex ] = (char)'\0';
        /*remove non-graph chars*/
        (void)inputFix( inputBuffer, cli::input::size );
        /*Validation : set the value for the response lookup table*/
        if ( 0 == strncmp( (const char*)inputBuffer, Q_CLI_DEFAULT_AT_COMMAND, cli::input::size ) ) {
            outRetval = cli::response::OK;
        }
        else if ( cli::response::NOT_FOUND != ( cliRetVal = exec( cli::input::storage ) ) ) {
            /*input is one of the subscribed commands*/
            outRetval = cliRetVal;
        }
        else if ( 0 == strncmp( (const char*)inputBuffer, Q_CLI_DEFAULT_ID_COMMAND, cli::input::size ) ) {
            outRetval = cli::response::DEVID;
        }
        else if ( strlen( (const char*)inputBuffer ) >= Q_CLI_MIN_INPUT_LENGTH ) {
            outRetval = cli::response::NOT_FOUND;
        }
        else {
            outRetval = cli::response::NO_RESPONSE; /*nothing to do*/
        }
        /*show the user output if available*/
        if ( nullptr != handler.Output ) {
            if ( (char)'\0' != handler.Output[ 0 ] ) {
                handleResponse( cli::response::OUTPUT );
            }
        }
        /*print out the command output*/
        handleResponse( outRetval );
        handler.Output[ 0 ] = (char)'\0';
        retValue = inputFlush(); /*flush buffers*/
    }
    

    return retValue;
}
/*============================================================================*/
char* cli::_Handler::getArgPtr( index_t n ) const
{
    char *retPtr = nullptr;

    if ( n > 0u ) {
        if ( cli::commandType::PARA  == Type ) {
            if ( 1u == n ) {
                retPtr = StrData;
            }
            else {
                index_t argc = 0u;

                --n;
                for ( index_t i = 0u ; (char)'\0' != StrData[ i ] ; ++i ) {
                    if ( instance->delim == StrData[ i ] ) {
                        if ( ++argc >= static_cast<index_t>( n ) ) {
                            retPtr = &StrData[ i + 1u ];
                            break;
                        }
                    }
                }
            }
        }
    }

    return retPtr;
}
/*============================================================================*/
int cli::_Handler::getArgInt( index_t n ) const
{
    return util::atoi( getArgPtr( n ) );
}
/*============================================================================*/
float32_t cli::_Handler::getArgFloat( index_t n ) const
{
    return util::atof( getArgPtr( n ) );
}
/*============================================================================*/
std::uint32_t cli::_Handler::getArgHex( index_t n ) const
{
    return util::xtou32( getArgPtr( n ) );
}
/*============================================================================*/
char* cli::_Handler::getArgString( index_t n, char *pOut ) const
{
    char *retPtr = nullptr;

    if ( ( nullptr != pOut ) && ( n > 0u ) ) {

        if ( cli::commandType::PARA == Type ) {
            index_t i, j, argc = 0u;

            --n;
            j = 0u;
            for ( i = 0u ; (char)'\0' != StrData[ i ] ; ++i ) {
                if ( argc == n ) {
                    retPtr = pOut;
                    if ( instance->delim == StrData[ i ] ) {
                        break;
                    }
                    pOut[ j++ ] = StrData[ i ];
                    pOut[ j ] = '\0';
                }
                if ( instance->delim == StrData[ i ] ) {
                    ++argc;
                }
            }
        }
    }

    return retPtr;
}
/*============================================================================*/
void cli::_Handler::output( const char c ) const
{
    instance->outputFcn( nullptr, c );
}
/*============================================================================*/
void cli::_Handler::output( const char *s ) const
{
    (void)util::outputString( instance->outputFcn, nullptr, s, false );
}
/*============================================================================*/