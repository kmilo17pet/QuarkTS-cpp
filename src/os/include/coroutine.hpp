#ifndef QOS_CPP_CO
#define QOS_CPP_CO 

#include "types.hpp"
#include "timer.hpp"

namespace qOS {
    namespace co {

        using coLabel_t = base_t;
        struct _coContext_s {
            coLabel_t label { 0 };
            qOS::timer delay;
        };
        using coContext = struct _coContext_s;

        inline void end_( void ) {}
        inline void nop_( void ) {}
        inline void reenter_( void ) {}
        inline void yield_( void ) {}
        inline void delay_( void ) {}
        inline void waitUntil_( void ) {}
        inline void restart_( void ) {}
    }
}

#ifdef __COUNTER__
    #define _co_label_ ( __COUNTER__ + 1 )
#else
    #define _co_label_ __LINE__
#endif
#define _co_empty_action    /*empty*/
#define _waitGetMacro( _1,_2,NAME,...) NAME

/*============================================================================*/
#define reenter  _co_reenter
// clang-format off
#define _co_reenter                                                            \
reenter_();                                                                    \
static qOS::co::coContext _cr;                                                 \
qOS::co::coContext *ctx = &_cr;                                                \
for ( qOS::co::coLabel_t *const _pc = &(ctx)->label; -1 != *_pc ; *_pc = -1 )  \
    if ( 0 ) {                                                                 \
        goto _co_continue_;                                                    \
        _co_continue_:                                                         \
        continue;                                                              \
    }                                                                          \
    else if ( 0 ) {                                                            \
        goto _co_break_;                                                       \
        _co_break_:                                                            \
        break;                                                                 \
    }                                                                          \
    else                                                                       \
        switch ( *_pc )                                                        \
            case 0 :                                                           \
// clang-format on

/*============================================================================*/
#define _co_cond( c )                                                          \
if ( !(c) ) {                                                                  \
    goto _co_break_;                                                           \
}                                                                              \

/*============================================================================*/
#define _co_t_cond( c )                                                        \
if ( !( (c) || _cr.delay.expired() ) ) {                                       \
    goto _co_break_;                                                           \
} 
/*============================================================================*/
// clang-format off
#define _co_save_restore( label, init_action, pos_label_action, end_action )   \
init_action;                                                                   \
for ( *_pc = (label) ;; )                                                      \
    if ( 0 ) {                                                                 \
        case ( label ) : {                                                     \
            pos_label_action                                                   \
            break;                                                             \
        }                                                                      \
    }                                                                          \
    else                                                                       \
        switch ( 0 )                                                           \
            for (;;)                                                           \
                if ( 1 )                                                       \
                    goto _co_continue_;                                        \
                else                                                           \
                    for (;;)                                                   \
                        if ( 1 )                                               \
                            goto _co_break_;                                   \
                        else /* falls through */                               \
                            case 0 : end_action                                \
// clang-format on

/*============================================================================*/
#define yield _co_yield(_co_label_)
#define _co_yield(label)                                                       \
yield_();                                                                      \
_co_save_restore( label, qOS::co::nop_(), _co_empty_action, _co_empty_action ) \

/*============================================================================*/
#define delay( t ) _co_delay(_co_label_ , t)
#define _co_delay( label, t )                                                  \
delay_();                                                                      \
_co_save_restore( label, _cr.delay.set(t) , _co_t_cond(0), qOS::co::end_() )   \

/*============================================================================*/
#define _waitUntil1( c ) _co_waitUntil(_co_label_ , c )
#define _co_waitUntil( label, c )                                              \
waitUntil_();                                                                  \
_co_save_restore( label, qOS::co::nop_(), _co_cond(c), qOS::co::end_() )       \

#define _timedWait2( c, t ) _co_timedWaitUntil(_co_label_ , c, t )
#define _co_timedWaitUntil( label, c, t )                                      \
waitUntil_();                                                             \
_co_save_restore( label, qOS::co::nop_(), _co_t_cond(c), qOS::co::end_() )     \

#define waitUntil(...) _waitGetMacro(__VA_ARGS__, _timedWait2, _waitUntil1 )(__VA_ARGS__)
/*============================================================================*/
#define restart _co_restart
#define _co_restart                                                            \
yield_();                                                                      \
*_pc = 0;                                                                      \
goto _co_break_                                                                \
/*============================================================================*/

#endif /*QOS_CPP_CO*/