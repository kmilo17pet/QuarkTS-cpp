#ifndef QOS_CPP_CO
#define QOS_CPP_CO

#include "include/types.hpp"
#include "include/timer.hpp"
#include "include/macro_overload.hpp"

namespace qOS {

    /**
    * @brief CoRoutines interfaces.
    */
    namespace co {

        /** \addtogroup  qcoroutines
        * @brief API interface for the @ref q_qcoroutines extension.
        *  @{
        */

        /** @brief The intrinsic type of co::position to hold a coroutine progress*/
        using state = base_t;
        /*! @cond  */

        /*cstat -MISRAC++2008-0-1-4_b*/
        const state UNDEFINED = -2;
        const state SUSPENDED = -1;
        const state BEGINNING =  0;
        /*cstat +MISRAC++2008-0-1-4_b*/

        class _coContext;
        /*! @endcond  */

        /** @brief A placeholder for the Co-Routine current position or progress*/
        class position final {
            state pos{ BEGINNING };
            /*! @cond  */
            public:
                state operator()(void) const
                {
                    return pos;
                }
                position& operator=( co::state l )
                {
                    pos = l;
                    return *this;
                }
            /*! @endcond  */
        };

        /** @brief A Co-Routine handle*/
        class handle {
            private:
                co::state prev = { co::UNDEFINED };
                _coContext *ctx{ nullptr };
                handle( handle const& ) = delete;
                void operator=( handle const& ) = delete;
            public:
                handle() = default;
                /**
                * @brief Try to execute the co::restart() statement externally.
                */
                void try_restart( void ) noexcept;
                /**
                * @brief Try to suspend the coroutine execution externally.
                */
                void try_suspend( void ) noexcept;
                /**
                * @brief Try to resume the coroutine execution externally after
                * a suspend operation.
                */
                void try_resume( void ) noexcept;
                /**
                * @brief Try to execute co::setPosition() statement externally.
                */
                void try_set( co::state p ) noexcept;
            friend class co::_coContext;
        };

        /** @brief A Co-Routine Semaphore*/
        class semaphore final {
            private:
                size_t count{ 1u };
                void signal( void ) noexcept;
                bool tryLock( void ) noexcept;
                semaphore( semaphore const& ) = delete;
                void operator=( semaphore const& ) = delete;
            public:
                /**
                * @brief Initializes a coroutine semaphore with a value for the counter. Internally,
                * the semaphores use an @c size_t to represent the counter, therefore
                * the @a init argument should be within range of this type.
                * @see co::semWait()
                * @see co::semSignal()
                * @param[in] init The initial count of the semaphore.
                */
                explicit semaphore( size_t init ) : count( init ) {}
                /**
                * @brief Set the coroutine semaphore with a value for the counter. Internally,
                * the semaphores use an @c size_t to represent the counter, therefore
                * the @a val argument should be within range of this type.
                * @see co::semWait()
                * @see co::semSignal()
                * @param[in] val The initial count of the semaphore.
                */
                void set( size_t val ) noexcept;
            friend class co::_coContext;
        };

        /*! @cond */
        /*cstat -MISRAC++2008-7-1-2*/
        class _coContext final {
            private:
                _coContext( _coContext const& ) = delete;
                void operator=( _coContext const& ) = delete;
                co::state label{ co::BEGINNING };
                qOS::timer tm;
            public:
                _coContext() = default;
                inline void saveHandle( co::handle& h ) noexcept
                {
                    h.ctx = this;
                }
                inline static void saveHandle( void ) noexcept {}
                inline static void semSignal( semaphore& s ) noexcept
                {
                    s.signal();
                }
                inline static bool semTrylock( semaphore& s ) noexcept
                {
                    return s.tryLock();
                }
                inline _coContext& operator=( co::state l )
                {
                    label = l;
                    return *this;
                }
                inline co::state operator()( void ) const
                {
                    return label;
                }
                inline void operator()( qOS::time_t t ) 
                {
                    (void)tm.set( t );
                }
                inline bool operator==( const co::state l ) const
                {
                    return l == label;
                }
                inline bool operator!=( const co::state l ) const
                {
                    return l != label;
                }
                inline bool timeout( void ) const 
                {
                    return tm.expired();
                }
            friend class handle;
        };
        /*cstat -MISRAC++2008-0-1-11*/

        inline void crNOP( void ) noexcept {}
        /*! @endcond */

        /**
        * @brief Defines a Coroutine segment. The co::reenter() statement is used 
        * to declare the starting point of a Coroutine. It should be placed at 
        * the start of the function in which the Coroutine runs.
        * @warning Only one segment is allowed inside a task.
        * 
        * Example:
        * @code{.c}
        * co::reenter() {
        *
        * }
        * @endcode
        */
        inline void reenter( void ) noexcept {}

        /**
        * @brief Defines a Coroutine segment with a supplied external handle.
        * The co::reenter() statement is used to declare the starting point of a
        * Coroutine. It should be placed at the start of the function in which 
        * the Coroutine runs.
        * @param[in] h The handle of a coroutine.
        * @warning Only one segment is allowed inside a task.
        * 
        * Example:
        * @code{.c}
        * co::reenter( handle ) {
        *
        * }
        * @endcode
        */
        inline void reenter( qOS::co::handle h ) noexcept { Q_UNUSED(h); }

        /**
        * @brief This statement is only allowed inside a Coroutine segment. 
        * co::yield return the CPU control back to the scheduler but saving the
        * execution progress. With the next task activation, the Coroutine will 
        * resume the execution after the last co::yield statement.
        * @verbatim Action sequence : [Save progress] then [Yield] @endverbatim
        */
        inline void yield( void ) noexcept {}

        /**
        * @brief Delay a coroutine for a given number of time.
        * @param[in] t The amount of time that the calling coroutine should yield.
        */
        inline void delay( qOS::time_t t ) noexcept { Q_UNUSED(t); }

        /**
        * @brief Yields until the logical condition is met.
        * @param[in] condition The logical condition to be evaluated. The 
        * condition determines if the blocking job ends (if condition is true) 
        * or continue yielding (if false)
        * @verbatim
        * Action sequence : [Save progress]
        *                 IF ( condition == False ) {
        *                     [Yield]
        *                 }
        * @endverbatim
        */
        inline void waitUntil( bool condition ) noexcept { Q_UNUSED(condition); }

        /**
        * @brief Yields until the logical condition is met or the specified 
        * timeout expires.
        * @param[in] condition The logical condition to be evaluated. The 
        * condition determines if the blocking job ends (if condition is @c true) 
        * or continue yielding (if false)
        * @param[in] timeout The specific amount of time to wait given in milliseconds..
        * @verbatim
        * Action sequence : [Save progress]
        *                 IF ( condition == False || NOT_EXPIRED(timeout) )
        *                     [Yield]
        *                 }
        * @endverbatim
        */
        inline void waitUntil( bool condition, qOS::time_t timeout ) noexcept { Q_UNUSED(condition); Q_UNUSED(timeout); }

        /**
        * @brief Check if the internal Co-routine timeout expires.
        * @return @c true when timer expires, otherwise, returns @c false.
        * Example:
        * @code{.c}
        * co::reenter( handle ) {
        *     co::waitUntil( count > 10 , 5.0f );
        *     if ( co::timeoutExpired() ) {
        *         co::restart;
        *     }
        * }
        * @endcode
        */
        inline void timeoutExpired( void ) noexcept {}

        /**
        * @brief This statement cause the running Coroutine to restart its 
        * execution at the place of the co::reenter() statement.
        * @verbatim Action sequence : [Reload progress] then [Yield] @endverbatim
        */
        inline void restart( void ) noexcept {}

        /**
        * @brief Carries out the "wait" operation on the semaphore. The wait
        * operation causes the Co-routine to block while the counter is zero. When
        * the counter reaches a value larger than zero, the Coroutine will continue.
        * @see co::semSignal()
        * @param[in] sem The co::semaphore object in which the operation is executed
        */
        inline void semWait( co::semaphore& sem ) noexcept { Q_UNUSED(sem); }

        /**
        * @brief Carries out the "signal" operation on the semaphore. The signal
        * operation increments the counter inside the semaphore, which eventually
        * will cause waiting Co-routines to continue executing.
        * @see co::semWait()
        * @param[in] sem The co::semaphore object in which the operation is executed
        * @return none.
        */
        inline void semSignal( co::semaphore& sem ) noexcept { Q_UNUSED(sem); }

        /**
        * @brief Labels the current position and saves it to @a var so it can be later
        * restored by co::setPosition()
        * @see co::setPosition()
        * @param[out] var The variable of type co::position where the current
        * position will be saved.
        */
        inline void getPosition( co::position &var ) noexcept { Q_UNUSED(var); }

        /**
        * @brief Restores the Co-Routine position saved in @a var
        * @see co::getPosition()
        * @param[in,out] var The variable of type co::position that contains the
        * position to be restored.
        */
        inline void setPosition( co::position &var ) noexcept { Q_UNUSED(var); }
        /*cstat +MISRAC++2008-0-1-11 +MISRAC++2008-7-1-2*/

        /** @}*/
    }
}
/*============================================================================*/
/*! @cond  */
#define _co_label_                                  ( __LINE__ )

/*============================================================================*/
#define reenter_0()                     _co_reenter( Q_NONE )
#define reenter_1(h)                    _co_reenter( h )
#define reenter(...)                    MACRO_OVERLOAD( reenter_ , __VA_ARGS__ )

/*============================================================================*/
// clang-format off
#define _co_reenter( h )                                                       \
reenter();                                                                     \
static qOS::co::_coContext _cr;                                                \
_cr.saveHandle( h );                                                           \
for ( ; _cr != qOS::co::SUSPENDED ; _cr = qOS::co::SUSPENDED )                 \
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
        switch ( _cr() )                                                       \
            case 0 :                                                           \

/*============================================================================*/
#define _coSaveRestore( label, init_action, pos_label_action )                 \
init_action;                                                                   \
for ( _cr = (label) ;; )                                                       \
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
if ( !( (c) || _cr.timeout() ) ) {                                             \
    goto _co_break_;                                                           \
} 
/*============================================================================*/
#define yield _co_yield(_co_label_)
#define _co_yield(label)                                                       \
yield();                                                                       \
_coSaveRestore( label, qOS::co::crNOP(), Q_NONE )                              \

/*============================================================================*/
#define delay( t ) _co_delay(_co_label_ , t)
#define _co_delay( label, t )                                                  \
delay(t);                                                                      \
_coSaveRestore( label, _cr(t) , _co_t_cond(0) )                                \

/*============================================================================*/
#define _wu_1( c ) _co_waitUntil(_co_label_ , c )
#define _co_waitUntil( label, c )                                              \
waitUntil(c);                                                                  \
_coSaveRestore( label, qOS::co::crNOP(), _co_cond(c) )                         \

#define _wu_2( c, t ) _co_timedWaitUntil(_co_label_ , c, t )
#define _co_timedWaitUntil( label, c, t )                                      \
waitUntil(c,t);                                                                \
_coSaveRestore( label, qOS::co::crNOP(), _co_t_cond(c) )                       \

#define waitUntil(...)                    MACRO_OVERLOAD( _wu_ , __VA_ARGS__ )
/*============================================================================*/
#define timeoutExpired()              timeoutExpired(), _cr.timeout()

/*============================================================================*/
#define restart _co_restart
#define _co_restart                                                            \
restart();                                                                     \
_cr = qOS::co::BEGINNING;                                                      \
goto _co_break_                                                                \

/*============================================================================*/
#define semWait( sem )                                                         \
semWait( sem );                                                                \
_coSaveRestore( _co_label_, qOS::co::crNOP(), _co_cond( _cr.semTrylock(sem)) ) \

/*============================================================================*/
#define semSignal( sem )                                                       \
semSignal( sem );                                                              \
_cr.semSignal( sem )                                                           \

/*============================================================================*/
#define getPosition( var )   _co_get_pos( var, _co_label_ )
#define _co_get_pos( var, label )                                              \
getPosition( var );                                                            \
var = label;                                                                   \
case ( label ) : qOS::co::crNOP()                                              \

/*============================================================================*/
#define setPosition( var )   co_res_pos( var, _co_label_ )
#define co_res_pos( var, label )                                               \
setPosition( var );                                                            \
_cr = var();                                                                   \
goto _co_break_                                                                \

/*! @endcond  */

#endif /*QOS_CPP_CO*/


