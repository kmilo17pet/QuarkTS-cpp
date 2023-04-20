#ifndef QOS_CPP_TYPES
#define QOS_CPP_TYPES

#if defined( ARDUINO ) || defined( ARDUINO_ARCH_AVR) || defined( ARDUINO_ARCH_SAMD ) || defined( ENERGIA_ARCH_MSP430ELF )
    #define ARDUINO_PLATFORM
#endif

#if ( __cplusplus < 201103L || defined ( __AVR_ARCH__ ) || defined( ARDUINO_PLATFORM ) || defined ( STM8Sxx ) )
    #include <stddef.h>
    #include <stdint.h>
    #include <stdlib.h>
    #include <string.h>
    #include <ctype.h>
    #include <limits.h>
#else 
    #include <cstddef>
    #include <cstdint>
    #include <cstdlib>
    #include <cstring>
    #include <cctype>
    #include <climits>
#endif

/** @addtogroup  qtypes
* @brief Sets of types having specified widths used to design the entire OS.
* It also define macros directives to perform specific actions and options
* for some OS APIs.
* @{
*/

using namespace std;

/**
* @brief A type to instantiate a integer-base variable. This type is
* compiler implementation-defined.
*/
using base_t = int;

/**
* @brief A type to instantiate a byte variable
*/
using byte_t = uint8_t;

/**
* @brief A type to instantiate an unsigned variable
*/
using unsigned_t = unsigned long;

/**
* @brief A type to instantiate a single-precision variable of 32-bits IEEE 754.
*/
using float32_t = float;  /*this is not always true in some compilers*/

/**
* @brief A type to instantiate a single-precision variable of 64-bits IEEE 754.
*/
using float64_t = double;  /*this is not always true in some compilers*/

/**
* @brief A type to instantiate an OS index variable. Can store the maximum
* size of a theoretically possible object of any type (including array).
* Should be used for array indexing and loop counting.
*/
using index_t = size_t;

/**
* @brief A type to instantiate a variable that hold the number of task cycles.
*/
using cycles_t = uint32_t;

/**
* @brief A type to instantiate a variable that hold the number of task iterations.
*/
using iteration_t = int32_t;

/**
* @brief A type to instantiate a variable to hold the priority value of a task.
*/
using priority_t = uint8_t;

/**
* @brief A type to instantiate a variable to hold a time count.
*/
using timeCount_t = unsigned long;

/*! @cond */
#define     Q_UNUSED(arg)     (void)(arg)
#define     Q_NONE            /*EMPTY MACRO*/
/*! @endcond */

/** @}*/

#endif /*QOS_CPP_TYPES*/
