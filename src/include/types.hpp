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
    #define STD_TYPE_UINT8_T    uint8_t
    #define STD_TYPE_UINT16_T   uint16_t
    #define STD_TYPE_UINT32_T   uint32_t
    #define STD_TYPE_INT8_T     int8_t
    #define STD_TYPE_INT16_T    int16_t
    #define STD_TYPE_INT32_T    int32_t
    #define STD_TYPE_UINTPTR_T  uintptr_t
    #define STD_TYPE_SIZE_T     size_t
#else
    #include <cstddef>
    #include <cstdint>
    #include <cstdlib>
    #include <cstring>
    #include <cctype>
    #include <climits>
    #define STD_TYPE_UINT8_T    std::uint8_t
    #define STD_TYPE_UINT16_T   std::uint16_t
    #define STD_TYPE_UINT32_T   std::uint32_t
    #define STD_TYPE_INT8_T     std::int8_t
    #define STD_TYPE_INT16_T    std::int16_t
    #define STD_TYPE_INT32_T    std::int32_t
    #define STD_TYPE_UINTPTR_T  std::uintptr_t
    #define STD_TYPE_SIZE_T     std::size_t
#endif

#if defined( ARDUINO_PLATFORM )
    #include <Arduino.h>
#else
    #include <string>
#endif

#ifndef SIZE_MAX
    #define SIZE_MAX ( ~static_cast<size_t>( 0 ) )
#endif

namespace qOS {
    /** @addtogroup  qtypes
    * @brief Sets of types having specified widths used to design the entire OS.
    * It also define macros directives to perform specific actions and options
    * for some OS APIs.
    * @{
    */

    /**
    * @brief A type to instantiate a integer-base variable. This size of this type is
    * implementation-defined.
    */
    using base_t = int;

    /**
    * @brief A type to instantiate a byte variable
    */
    using byte_t = STD_TYPE_UINT8_T;

    /**
    * @brief A type to instantiate an unsigned variable
    */
    using unsigned_t = unsigned long;

    /**
    * @brief A type to instantiate an signed variable
    */
    using signed_t = long int;

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
    using index_t = STD_TYPE_SIZE_T;

    /**
    * @brief A type to instantiate a variable that hold the number of task cycles.
    */
    using cycles_t = STD_TYPE_UINT32_T;

    /**
    * @brief A type to instantiate a variable that hold the number of task iterations.
    */
    using iteration_t = STD_TYPE_INT32_T;

    /**
    * @brief A type to instantiate a variable to hold the priority value of a task.
    */
    using priority_t = STD_TYPE_UINT8_T;

    /*! @cond */
    using uint8_t = STD_TYPE_UINT8_T;
    using uint16_t = STD_TYPE_UINT16_T;
    using uint32_t = STD_TYPE_UINT32_T;
    using int8_t = STD_TYPE_INT8_T;
    using int16_t = STD_TYPE_INT16_T;
    using int32_t = STD_TYPE_INT32_T;
    using uintptr_t = STD_TYPE_UINTPTR_T;
    using size_t = STD_TYPE_SIZE_T;
    #if defined( ARDUINO_PLATFORM )
        using string = String;
    #else
        using string = std::string;
    #endif
    /*! @endcond */

    /**
    * @brief A type to instantiate a variable to hold a time count.
    */
    using timeCount_t = unsigned long;

    /** @}*/
}

/*! @cond */
#define     Q_UNUSED(arg)     (void)(arg)
#define     Q_NONE            /*EMPTY MACRO*/
/*! @endcond */

#endif /*QOS_CPP_TYPES*/
