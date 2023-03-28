#ifndef QOS_CPP_CO
#define QOS_CPP_CO

#include "types.hpp"
#include "timer.hpp"
#include "macroutil.hpp"

namespace qOS {
    namespace co {

        enum state : base_t {
            UNDEFINED = -2,
            SUSPENDED = -1,
            BEGINNING =  0,
        };
        class _coContext;

        class position {
            public:
            base_t pos{ 0 };
        };

        class handle {
            protected:
                co::state prev = { co::state::UNDEFINED };
                _coContext *ctx{ nullptr };
                handle( handle const& ) = delete;      /* not copyable*/
                void operator=( handle const& ) = delete;  /* not assignable*/
            public:
                handle() = default;
                void try_restart( void );
                void try_suspend( void );
                void try_resume( void );
                void try_set( co::state p );
            friend class co::_coContext;
        };

        class semaphore {
            protected:
                std::size_t count{ 1u };
                void signal( void );
                bool tryLock( void );
                semaphore( semaphore const& ) = delete;      /* not copyable*/
                void operator=( semaphore const& ) = delete;  /* not assignable*/
            public:
                semaphore( std::size_t init ) : count( init ) {}
                void set( std::size_t val );
            friend class co::_coContext;
        };

        class _coContext {
            protected:
                _coContext( _coContext const& ) = delete;      /* not copyable*/
                void operator=( _coContext const& ) = delete;  /* not assignable*/
            public:
                _coContext() = default;
                co::state label{ co::state::BEGINNING };
                qOS::timer delay;
                inline void saveHandle( co::handle& h )
                {
                    h.ctx = this;
                }
                inline void saveHandle( void ) { }
                inline void semSignal( semaphore& s )
                {
                    s.signal();
                }
                inline bool semTrylock( semaphore& s )
                {
                    return s.tryLock();
                }
        };

        inline void end_( void ) {}
        inline void nop_( void ) {}
        inline void reenter( void ) {}
        inline void reenter( qOS::co::handle h ) {}
        inline void yield( void ) {}
        inline void delay( qOS::time_t ) {}
        inline void waitUntil( bool condition ) {}
        inline void waitUntil( bool condition, qOS::time_t timeout ) {}
        inline void restart( void ) {}
        inline void semWait( co::semaphore& sem ) {}
        inline void semSignal( co::semaphore& sem ) {}
        inline void getPosition( co::position &var ) {}
        inline void setPosition( co::position &var ) {}
    }
}
/*============================================================================*/
#define _co_label_ static_cast<qOS::co::state>( __LINE__ )

/*============================================================================*/
#define reenter(...)                OVERLOADED_MACRO( _reenter, __VA_ARGS__ )
#define _reenter0()                 _co_reenter_intro( Q_NONE )
#define _reenter1( Handle )         _co_reenter_intro( Handle )

/*============================================================================*/
#define _co_reenter_intro( h )                                                 \
reenter();                                                                     \
static qOS::co::_coContext _cr;                                                \
qOS::co::_coContext *ctx = &_cr;                                               \
_cr.saveHandle( h );                                                           \
_co_reenter                                                                    \

/*============================================================================*/
// clang-format off
#define _co_reenter                                                            \
for ( qOS::co::state *const _pc = &(ctx)->label;                               \
      qOS::co::state::SUSPENDED != *_pc ;                                      \
      *_pc = qOS::co::state::SUSPENDED )                                       \
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
yield();                                                                       \
_co_save_restore( label, qOS::co::nop_(), Q_NONE, Q_NONE )                     \

/*============================================================================*/
#define delay( t ) _co_delay(_co_label_ , t)
#define _co_delay( label, t )                                                  \
delay(t);                                                                      \
_co_save_restore( label, _cr.delay.set(t) , _co_t_cond(0), qOS::co::end_() )   \

/*============================================================================*/
#define waitUntil(...)              OVERLOADED_MACRO( _waitUntil, __VA_ARGS__ )

#define _waitUntil1( c )                                                       \
waitUntil(c);                                                                  \
_co_save_restore( _co_label_, qOS::co::nop_(), _co_cond(c), qOS::co::end_() )  \

#define _waitUntil2( c, t )                                                    \
waitUntil(c,t);                                                                \
_co_save_restore( _co_label_, qOS::co::nop_(), _co_t_cond(c), qOS::co::end_() )\

/*============================================================================*/
#define restart _co_restart
#define _co_restart                                                            \
restart();                                                                     \
*_pc = qOS::co::state::BEGINNING;                                              \
goto _co_break_                                                                \

/*============================================================================*/
#define semWait( sem )                                                         \
semWait( sem );                                                                \
_co_save_restore( _co_label_, qOS::co::nop_(), _co_cond( _cr.semTrylock(sem)), \
                  qOS::co::end_() )                                            \

/*============================================================================*/
#define semSignal( sem )                                                       \
semSignal( sem );                                                              \
_cr.semSignal( sem )                                                           \

/*============================================================================*/
#define getPosition( var )   _co_get_pos( var, _co_label_ )
#define _co_get_pos( var, label )                                              \
getPosition( var );                                                            \
var.pos = label;                                                               \
case ( label ) : qOS::co::nop_()                                               \

/*============================================================================*/
#define setPosition( var )   co_res_pos( var, _co_label_ )
#define co_res_pos( var, label )                                               \
setPosition( var );                                                            \
for ( *_pc = static_cast<qOS::co::state>( var.pos ) ;; )                       \
    if ( 1 )                                                                   \
        switch ( 0 )                                                           \
            for (;;)                                                           \
                if ( 1 )                                                       \
                    goto _co_continue_;                                        \
                else                                                           \
                    for (;;)                                                   \
                        if ( 1 )                                               \
                            goto _co_break_;                                   \
                        else /* falls through */                               \
                            case 0 : qOS::co::end_()                           \


#endif /*QOS_CPP_CO*/


