#ifndef QOS_CPP_CO
#define QOS_CPP_CO

#include "types.hpp"
#include "timer.hpp"

namespace qOS {
    namespace co {

        using state = base_t;
        const state UNDEFINED = -2;
        const state SUSPENDED = -1;
        const state BEGINNING =  0; 
        
        class _coContext;

        class position {
            public:
            state pos{ BEGINNING };
        };

        class handle {
            protected:
                co::state prev = { co::UNDEFINED };
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
                co::state label{ co::BEGINNING };
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

        inline void nop( void ) {}
        inline void reenter( void ) {}
        inline void reenter( qOS::co::handle h ) { Q_UNUSED(h); }
        inline void yield( void ) {}
        inline void delay( qOS::time_t t ) { Q_UNUSED(t); }
        inline void waitUntil( bool condition ) { Q_UNUSED(condition); }
        inline void waitUntil( bool condition, qOS::time_t timeout ) { Q_UNUSED(condition); Q_UNUSED(timeout); }
        inline void restart( void ) {}
        inline void semWait( co::semaphore& sem ) { Q_UNUSED(sem); }
        inline void semSignal( co::semaphore& sem ) { Q_UNUSED(sem); }
        inline void getPosition( co::position &var ) { Q_UNUSED(var); }
        inline void setPosition( co::position &var ) { Q_UNUSED(var); }
    }
}
/*============================================================================*/
#define _co_label_                                  ( __LINE__ )

/*============================================================================*/
#define reenter(...)    _reenter(__VA_ARGS__, _reenter0, _reenter1)(__VA_ARGS__)
#define _reenter(_1, _reenter1, _reenter0, ...)    _reenter0
#define _reenter1( Handle )                        _co_reenter_impl( Handle )
#define _reenter0                             _co_reenter_impl( Q_NONE )


/*============================================================================*/
#define _co_reenter_impl( h )                                                 \
reenter();                                                                     \
static qOS::co::_coContext _cr;                                                \
qOS::co::_coContext *ctx = &_cr;                                               \
_cr.saveHandle( h );                                                           \
_co_reenter                                                                    \

/*============================================================================*/
// clang-format off
#define _co_reenter                                                            \
for ( qOS::co::state *const _pc = &(ctx)->label;                               \
      qOS::co::SUSPENDED != *_pc ;                                             \
      *_pc = qOS::co::SUSPENDED )                                              \
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

/*============================================================================*/
#define _co_save_restore( label, init_action, pos_label_action )               \
init_action;                                                                   \
for ( *_pc = (label) ;; )                                                      \
    if ( 0 ) {                                                                 \
        case ( label ) : {                                                     \
            pos_label_action                                                   \
            break;                                                             \
        }                                                                      \
    }                                                                          \
    else goto _co_break_                                                       \
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
#define yield _co_yield(_co_label_)
#define _co_yield(label)                                                       \
yield();                                                                       \
_co_save_restore( label, qOS::co::nop(), Q_NONE )                              \

/*============================================================================*/
#define delay( t ) _co_delay(_co_label_ , t)
#define _co_delay( label, t )                                                  \
delay(t);                                                                      \
_co_save_restore( label, _cr.delay.set(t) , _co_t_cond(0) )                    \

/*============================================================================*/
#define _wu1( c ) _co_waitUntil(_co_label_ , c )
#define _co_waitUntil( label, c )                                              \
waitUntil(c);                                                                  \
_co_save_restore( label, qOS::co::nop(), _co_cond(c) )                         \

#define _wu2( c, t ) _co_timedWaitUntil(_co_label_ , c, t )
#define _co_timedWaitUntil( label, c, t )                                      \
waitUntil(c,t);                                                                \
_co_save_restore( label, qOS::co::nop(), _co_t_cond(c) )                       \

#define _waitGetMacro( _1,_2,NAME,...) NAME
#define waitUntil(...) _waitGetMacro(__VA_ARGS__, _wu2, _wu1 )(__VA_ARGS__)

/*============================================================================*/
#define restart _co_restart
#define _co_restart                                                            \
restart();                                                                     \
*_pc = qOS::co::BEGINNING;                                                     \
goto _co_break_                                                                \

/*============================================================================*/
#define semWait( sem )                                                         \
semWait( sem );                                                                \
_co_save_restore( _co_label_, qOS::co::nop(), _co_cond( _cr.semTrylock(sem)) ) \

/*============================================================================*/
#define semSignal( sem )                                                       \
semSignal( sem );                                                              \
_cr.semSignal( sem )                                                           \

/*============================================================================*/
#define getPosition( var )   _co_get_pos( var, _co_label_ )
#define _co_get_pos( var, label )                                              \
getPosition( var );                                                            \
var.pos = label;                                                               \
case ( label ) : qOS::co::nop()                                                \

/*============================================================================*/
#define setPosition( var )   co_res_pos( var, _co_label_ )
#define co_res_pos( var, label )                                               \
setPosition( var );                                                            \
*_pc = var.pos;                                                                \
goto _co_break_                                                                \


#endif /*QOS_CPP_CO*/


