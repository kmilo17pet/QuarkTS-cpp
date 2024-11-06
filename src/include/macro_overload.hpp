#ifndef QOS_CPP_MACRO_OVERLOAD
#define QOS_CPP_MACRO_OVERLOAD

    #define QOS_MO_CAT(a, ...)                      QOS_MO_PRIMITIVE_CAT( a, __VA_ARGS__ )
    #define QOS_MO_PRIMITIVE_CAT(a, ...)            a ## __VA_ARGS__
    #define QOS_MO_SPLIT(i, ...)                    QOS_MO_PRIMITIVE_CAT( QOS_MO_SPLIT_, i )( __VA_ARGS__ )
    #define QOS_MO_SPLIT_0(a, ...)                  a
    #define QOS_MO_SPLIT_1(a, ...)                  __VA_ARGS__
    #define QOS_IS_VARIADIC(...)                    QOS_MO_SPLIT( 0, QOS_MO_CAT( QOS_IS_VARIADIC_R_, QOS_IS_VARIADIC_C __VA_ARGS__ ) )
    #define QOS_IS_VARIADIC_C(...)                  1
    #define QOS_IS_VARIADIC_R_1                     1,
    #define QOS_IS_VARIADIC_R_QOS_IS_VARIADIC_C     0,
    #define QOS_MO_IIF(bit)                         QOS_MO_PRIMITIVE_CAT( QOS_MO_IIF_, bit )
    #define QOS_MO_IIF_0(t, ...)                    __VA_ARGS__
    #define QOS_MO_IIF_1(t, ...)                    t
    #define QOS_MO_IS_EMPTY_NON_FUNCTION(...)       QOS_MO_IIF( QOS_IS_VARIADIC( __VA_ARGS__ ) )( 0, QOS_IS_VARIADIC( QOS_MO_IS_EMPTY_NON_FUNCTION_C __VA_ARGS__ () ) )
    #define QOS_MO_IS_EMPTY_NON_FUNCTION_C()        ()
    #define QOS_MO_COMMA()                          ,
    #define QOS_MO_REM(...)                         __VA_ARGS__
    #define QOS_MO_SIZE(...)                        QOS_MO_SPLIT( 0, QOS_MO_SPLIT( 1, QOS_MO_SIZE_A( QOS_MO_COMMA, QOS_MO_REM( __VA_ARGS__ ) ),, ) )
    #define QOS_MO_SIZE_A(_, im)                    QOS_MO_SIZE_B( im, _() 5, _() 4, _() 3, _() 2, _() 1,)
    #define QOS_MO_SIZE_B(a, b, c, d, e, _, ...)    _
    #define QOS_MO_GTZ_OVERLOAD(prefix, ...)        QOS_MO_CAT( prefix, QOS_MO_SIZE( __VA_ARGS__) )
    #define QOS_MO_SELECT(bit, t, f)                QOS_MO_SELECT_I( bit, t, f )
    #define QOS_MO_SELECT_I(bit, t, f)              QOS_MO_SELECT_ ## bit( t, f )
    #define QOS_MO_SELECT_0(t, f)                   f
    #define QOS_MO_SELECT_1(t, f)                   t

    #define MACRO_OVERLOAD( prefix, ... )           \
    QOS_MO_SELECT( QOS_MO_IS_EMPTY_NON_FUNCTION( __VA_ARGS__ ), QOS_MO_CAT( prefix , 0() ), QOS_MO_GTZ_OVERLOAD( prefix , __VA_ARGS__ )( __VA_ARGS__ ) )

#endif