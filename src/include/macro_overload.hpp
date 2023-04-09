#ifndef QOS_CPP_MACRO_OVERLOAD
#define QOS_CPP_MACRO_OVERLOAD

    #define _CAT(a, ...)                    _PRIMITIVE_CAT( a, __VA_ARGS__ )
    #define _PRIMITIVE_CAT(a, ...)          a ## __VA_ARGS__
    #define _SPLIT(i, ...)                  _PRIMITIVE_CAT( _SPLIT_, i )( __VA_ARGS__ )
    #define _SPLIT_0(a, ...)                a
    #define _SPLIT_1(a, ...)                __VA_ARGS__
    #define IS_VARIADIC(...)                _SPLIT( 0, _CAT( IS_VARIADIC_R_, IS_VARIADIC_C __VA_ARGS__ ) )
    #define IS_VARIADIC_C(...)              1
    #define IS_VARIADIC_R_1                 1,
    #define IS_VARIADIC_R_IS_VARIADIC_C     0,
    #define _IIF(bit)                       _PRIMITIVE_CAT( _IIF_, bit )
    #define _IIF_0(t, ...)                  __VA_ARGS__
    #define _IIF_1(t, ...)                  t
    #define _IS_EMPTY_NON_FUNCTION(...)     _IIF( IS_VARIADIC( __VA_ARGS__ ) )( 0, IS_VARIADIC( _IS_EMPTY_NON_FUNCTION_C __VA_ARGS__ () ) )
    #define _IS_EMPTY_NON_FUNCTION_C()      ()
    #define _COMMA()                        ,
    #define _REM(...)                       __VA_ARGS__
    #define _SIZE(...)                      _SPLIT( 0, _SPLIT( 1, _SIZE_A( _COMMA, _REM( __VA_ARGS__ ) ),, ) )
    #define _SIZE_A(_, im)                  _SIZE_B( im, _() 5, _() 4, _() 3, _() 2, _() 1,)
    #define _SIZE_B(a, b, c, d, e, _, ...)  _
    #define _GTZ_OVERLOAD(prefix, ...)      _CAT( prefix, _SIZE( __VA_ARGS__) )
    #define _SELECT(bit, t, f)              _SELECT_I( bit, t, f )
    #define _SELECT_I(bit, t, f)            _SELECT_ ## bit( t, f )
    #define _SELECT_0(t, f)                 f
    #define _SELECT_1(t, f)                 t

    #define MACRO_OVERLOAD( prefix, ... )   _SELECT( _IS_EMPTY_NON_FUNCTION( __VA_ARGS__ ), _CAT( prefix , 0() ), _GTZ_OVERLOAD( prefix , __VA_ARGS__ )( __VA_ARGS__ ) )

#endif