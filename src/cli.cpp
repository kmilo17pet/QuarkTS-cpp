#include "include/cli.hpp"

using namespace qOS;

static const char * const Q_CLI_DEFAULT_AT_COMMAND = "at";
static const char * const Q_CLI_DEFAULT_ID_COMMAND = "atid";
static const char * const Q_CLI_DEFAULT_NOTALLOWED_RSP_STRING = ":NOT ALLOWED";
static const char * const Q_CLI_DEFAULT_DEVID_STRING = "QuarkTS CLI";
static const size_t Q_CLI_MIN_INPUT_LENGTH = 3U;
static const size_t Q_CLI_RECOMMENDED_INPUT_SIZE = 128U;
static size_t CMD_MASK_ARG_MAX_NUM( cli::options_t opt );
static size_t CMD_MASK_ARG_MIN_NUM( cli::options_t opt );

/*============================================================================*/
static size_t CMD_MASK_ARG_MAX_NUM( cli::options_t opt )
{
    return ( static_cast<size_t>(opt) >> 4U ) & static_cast<size_t>( 0x000FU );
}
/*============================================================================*/
static size_t CMD_MASK_ARG_MIN_NUM( cli::options_t opt )
{
    return static_cast<size_t>(opt)  & static_cast<size_t>( 0x000FU );
}
/*============================================================================*/
static char* inputFix( char *s, size_t maxlen )
{
    int j = 0;
    int noL = 0;

    for ( int i = 0 ; ( '\0' != s[ i ] ) && ( maxlen > 0U ) ; ++i ) {
        if ( ( '=' == s[ i ] ) || ( '?' == s[ i ] ) ) {
            noL = 1;
        }
        if ( '\r' == s[ i ] ) {
            s[ i ] = '\0';
            break;
        }
        if ( 0 != isgraph( static_cast<int>( s[ i ] ) ) ) {
            if ( 0 == noL ) {
              s[ j++ ] = static_cast<char>( tolower( static_cast<int>( s[ i ] ) ) );
            }
            else {
                s[ j++ ] = s[ i ];
            }
        }
        --maxlen;
    }
    s[ j ] = '\0';

    return s;
}
/*============================================================================*/
void cli::input::flush( void )
{
    (void)memset( storage, 0, size );
}
/*============================================================================*/
bool commandLineInterface::setup( util::putChar_t outFcn, char *pInput, const size_t sizeIn, char *pOutput, const size_t sizeOut )
{
    bool retValue = false;

    if ( nullptr != outFcn ) {
        outputFcn = outFcn;
        sizeOutput = sizeOut;
        handler.output = pOutput;
        handler.instance = this;
        id_rsp = Q_CLI_DEFAULT_DEVID_STRING;
        cli::input::storage = pInput;
        cli::input::size = sizeIn;
        cli::input::maxIndex = sizeIn - 1U;
        (void)memset( handler.output, 0, this->sizeOutput );
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
        if ( cmd.cmdLen >= 2U ) {
            /*command should start with an <at> at the beginning */
            if ( ( 'a' == textCommand[ 0 ] ) && ( 't' == textCommand[ 1 ] ) ) {
                cmd.Text = textCommand;
                cmd.cmdCallback = cFcn; /*install the callback*/
                cmd.cmdOpt = 0x0FFFU & cmdOpt; /*high nibble not used yet*/
                cmd.param = param;
                retValue = subscribed.insert( &cmd );
            }
        }
    }

    return retValue;
}
/*============================================================================*/
bool commandLineInterface::notify( void )
{
    cli::input::ready = true;
    cli::input::index = 0U;
    if ( nullptr != xNotifyFcn ) {
        xNotifyFcn( this ); /*external task notification if available*/
    }

    return true;
}
/*============================================================================*/
bool commandLineInterface::isrHandler( const char c )
{
    bool retValue = false;
    const bool readyInput = cli::input::ready;
    /*isgraph is known to have no-side effects*/
    /*check if the input is available and incoming chars are valid*/
    /*cstat -MISRAC++2008-5-14-1*/
    if ( ( !readyInput ) && ( 0 != isgraph( static_cast<int>( c ) ) ) ) {
        /*to avoid undefined order of volatile accesses*/
        index_t currentIndex = cli::input::index;
        /*cstat -CERT-INT30-C_a*/
        cli::input::storage[ currentIndex++ ] = c; /*insert char*/
        /*cstat +CERT-INT30-C_a*/
        /*put the null-char after to keep the string safe*/
        cli::input::storage[ currentIndex ] = '\0';
        /*check if the input buffer its reached*/
        if ( currentIndex >= cli::input::maxIndex ) {
            currentIndex = 0U;
        }
        cli::input::index = currentIndex;
    }
    /*cstat +MISRAC++2008-5-14-1*/
    if ( '\r' == c ) {
        retValue = notify(); /*end of line received, send the notification*/
    }

    return retValue;
}
/*============================================================================*/
bool commandLineInterface::isrHandler( char *pData, const size_t n )
{
    bool retValue = false;
    const bool readyInput = cli::input::ready;
    const size_t maxToInsert = cli::input::maxIndex;

    if ( ( n > 0U ) && ( n < maxToInsert ) &&  ( !readyInput ) ) {
        if ( 1U == n ) {
            retValue = isrHandler( pData[ 0 ] );
        }
        else {
            if ( 0 != isgraph( static_cast<int>( pData[ 0 ] ) ) ) {
                /*find the end of line safely*/
                if ( nullptr != util::strchr( pData, static_cast<int>( '\r' ), maxToInsert ) ) {
                    (void)util::strcpy( cli::input::storage, pData, maxToInsert );
                    retValue = notify();
                }
            }
        }
    }

    return retValue;
}
/*============================================================================*/
size_t commandLineInterface::numOfArgs( const char *str ) const
{
    size_t count = 0U;

    while ( '\0' != *str ) {
        if ( delim == *str++ ) {
            ++count;
        }
    }

    return ( count + 1U );
}
/*============================================================================*/
bool commandLineInterface::preProcessing( cli::command *cmd, char *inputBuffer )
{
    bool retValue = false;

    handler.Type = cli::commandType::UNDEF;
    handler.Command = cmd;
    handler.StrLen = util::strlen( const_cast<const char*>( inputBuffer ), Q_CLI_RECOMMENDED_INPUT_SIZE ) - cmd->cmdLen;
    handler.StrData = const_cast<char*>( &inputBuffer[ cmd->cmdLen ] );
    handler.NumArgs = 0U;

    if ( 0U == handler.StrLen ) { /*command should be an ACT command */
        if ( 0U != ( cmd->cmdOpt & static_cast<cli::options_t>( cli::commandType::ACT ) ) ) {
            handler.Type = cli::commandType::ACT;  /*set the type to ACT*/
            retValue = true;
        }
    }
    else {
        if ( '?' == handler.StrData[ 0 ] ) { /*command should be READ command */
            if ( 0U != ( cmd->cmdOpt & static_cast<cli::options_t>( cli::commandType::READ ) ) ) {
                handler.Type = cli::commandType::READ; /*set the type to READ*/
                ++handler.StrData; /*move string pointer once*/
                --handler.StrLen;  /*decrease the len one*/
                retValue = true;
            }
        }
        else if ( handler.StrLen >= 2U ) { /*can be at+xx=? or at+xx=...*/
            if ( '=' == handler.StrData[ 0 ] ) { /*could be a TEST or PARA command*/
                if ( '?' == handler.StrData[ 1 ] ) {
                    if ( ( 2U == handler.StrLen ) && ( 0U != ( cmd->cmdOpt & static_cast<cli::options_t>( cli::commandType::TEST ) ) ) ) {
                        /*command should be a TEST Command*/
                        handler.Type = cli::commandType::TEST; /*set the type to TEST*/
                        handler.StrData += 2; /*move string two positions ahead*/
                        handler.StrLen -= 2U;  /*decrease the len*/
                        retValue = true;
                    }
                }
                else { /*definitely is a PARA command*/
                    if ( 0U != ( cmd->cmdOpt & static_cast<cli::options_t>( cli::commandType::PARA ) ) ) { /*check if is allowed*/
                        const size_t argMin = CMD_MASK_ARG_MIN_NUM( cmd->cmdOpt );
                        const size_t argMax = CMD_MASK_ARG_MAX_NUM( cmd->cmdOpt );
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
        const bool readyInput = cli::input::ready;
        const size_t maxToInsert = cli::input::maxIndex;
        /*util::strLen is known to have no side effects*/
        /*cstat -MISRAC++2008-5-14-1*/
        if ( ( !readyInput ) && ( util::strlen( cmd, maxToInsert ) <= maxToInsert ) ) {
            (void)util::strcpy( cli::input::storage, cmd, maxToInsert );
            (void)inputFix( cli::input::storage, maxToInsert );
            retValue = notify();
        }
        /*cstat +MISRAC++2008-5-14-1*/
    }

    return retValue;
}
/*============================================================================*/
cli::response commandLineInterface::exec( const char *cmd )
{
    cli::response retValue = cli::response::NOT_FOUND;

    if ( nullptr != cmd ) {
        for ( auto i = subscribed.begin(); i.until(); i++ ) {
            cli::command * const iCmd = i.get<cli::command*>();
            /*check if the input matches the subscribed command */
            if ( 0 == strncmp( cmd, iCmd->Text, iCmd->cmdLen ) ) {
                retValue = cli::response::NOT_ALLOWED;
                if ( preProcessing( iCmd, const_cast<char*>( cmd ) ) ) {
                    /*if success, proceed with the user pos-processing*/
                    if ( cli::commandType::UNDEF == handler.Type ) {
                        retValue = cli::response::ERROR;
                    }
                    else {
                        retValue = iCmd->cmdCallback( handler ); /*invoke the callback*/
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
    cli::input::index = 0U;
    cli::input::storage[ 0 ] = '\0';

    return true;
}
/*============================================================================*/
void commandLineInterface::handleResponse( cli::response retval )
{
    if ( cli::response::NO_RESPONSE != retval ) {

        switch ( retval ) { /*handle the command-callback response*/
            case cli::response::ERROR:
                (void)util::outputString( outputFcn, er_rsp );
                break;
            case cli::response::OK:
                (void)util::outputString( outputFcn, ok_rsp );
                break;
            case cli::response::NOT_ALLOWED:
                (void)util::outputString( outputFcn, er_rsp );
                (void)util::outputString( outputFcn, Q_CLI_DEFAULT_NOTALLOWED_RSP_STRING );
                break;
            case cli::response::DEVID:
                (void)util::outputString( outputFcn, id_rsp );
                break;
            case cli::response::NOT_FOUND:
                (void)util::outputString( outputFcn, nf_rsp );
                break;
            case cli::response::OUTPUT_RESPONSE:
                (void)util::outputString( outputFcn, handler.output );
                break;
            default: /*AT_ERROR_CODE(#) */
                if ( static_cast<base_t>( retval ) < 0 ) {
                    const int32_t errorCode = cli::ERROR_CODE( static_cast<int16_t>( retval ) );

                    (void)util::integerToString( errorCode, handler.output, 10U );
                    (void)util::outputString( outputFcn, er_rsp );
                    outputFcn( nullptr, ':' );
                    (void)util::outputString( outputFcn, handler.output );
                    handler.output[ 0 ] = '\0';
                }
                break;
        }
        (void)util::outputString( outputFcn, eol );
    }
}
/*============================================================================*/
bool commandLineInterface::run( void )
{
    bool retValue = false;

    if ( cli::input::ready ) { /*a new input has arrived*/
        cli::response outRetval;
        cli::response cliRetVal;
        /*cstat -MISRAC++2008-7-1-1*/
        char *inputBuffer = cli::input::storage;
        /*cstat +MISRAC++2008-7-1-1*/
        inputBuffer[ cli::input::maxIndex ] = '\0';
        /*remove non-graph chars*/
        (void)inputFix( inputBuffer, cli::input::size );
        /*Validation : set the value for the response lookup table*/
        /*cstat -CERT-STR32-C*/
        if ( 0 == strncmp( const_cast<const char*>( inputBuffer ), Q_CLI_DEFAULT_AT_COMMAND, cli::input::size ) ) {
            outRetval = cli::response::OK;
        }
        /*cstat -MISRAC++2008-6-2-1*/
        else if ( cli::response::NOT_FOUND != ( cliRetVal = exec( cli::input::storage ) ) ) {
            /*input is one of the subscribed commands*/
            outRetval = cliRetVal;
        }
        /*cstat +MISRAC++2008-6-2-1*/
        else if ( 0 == strncmp( const_cast<const char*>( inputBuffer ), Q_CLI_DEFAULT_ID_COMMAND, cli::input::size ) ) {
            outRetval = cli::response::DEVID;
        }
        else if ( util::strlen( const_cast<const char*>( inputBuffer ), cli::input::size ) >= Q_CLI_MIN_INPUT_LENGTH ) {
            outRetval = cli::response::NOT_FOUND;
        }
        /*cstat +CERT-STR32-C*/
        else {
            outRetval = cli::response::NO_RESPONSE; /*nothing to do*/
        }
        /*show the user output if available*/
        if ( nullptr != handler.output ) {
            if ( '\0' != handler.output[ 0 ] ) {
                handleResponse( cli::response::OUTPUT_RESPONSE );
            }
        }
        handleResponse( outRetval ); /*print out the command output*/
        handler.output[ 0 ] = '\0';
        retValue = inputFlush(); /*flush buffers*/
    }
    

    return retValue;
}
/*============================================================================*/
char* cli::commandHandler::getArgPtr( index_t n ) const
{
    char *retPtr = nullptr;

    if ( n > 0U ) {
        if ( cli::commandType::PARA  == Type ) {
            if ( 1U == n ) {
                retPtr = StrData;
            }
            else {
                index_t argc = 0U;

                --n;
                for ( index_t i = 0U ; '\0' != StrData[ i ] ; ++i ) {
                    if ( instance->delim == StrData[ i ] ) {
                        if ( ++argc >= static_cast<index_t>( n ) ) {
                            retPtr = &StrData[ i + 1U ];
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
int cli::commandHandler::getArgInt( index_t n ) const
{
    return util::stringToInt( getArgPtr( n ) );
}
/*============================================================================*/
float32_t cli::commandHandler::getArgFloat( index_t n ) const
{
    /*cstat -MISRAC++2008-5-0-6 -MISRAC++2008-5-0-3 -CERT-FLP34-C*/
    return static_cast<float32_t>( util::stringToFloat( getArgPtr( n ) ) );
    /*cstat +MISRAC++2008-5-0-6 +MISRAC++2008-5-0-3 +CERT-FLP34-C*/
}
/*============================================================================*/
uint32_t cli::commandHandler::getArgHex( index_t n ) const
{
    return util::hexStringToUnsigned( getArgPtr( n ) );
}
/*============================================================================*/
char* cli::commandHandler::getArgString( index_t n, char *pOut )
{
    char *retPtr = nullptr;

    if ( ( nullptr != pOut ) && ( n > 0U ) ) {

        if ( cli::commandType::PARA == Type ) {
            index_t j;
            index_t argc = 0U;

            --n;
            j = 0U;
            for ( index_t i = 0U ; '\0' != StrData[ i ] ; ++i ) {
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
void cli::commandHandler::writeOut( const char c ) const
{
    instance->outputFcn( nullptr, c );
}
/*============================================================================*/
void cli::commandHandler::writeOut( const char *s ) const
{
    (void)util::outputString( instance->outputFcn, s );
}
/*============================================================================*/
