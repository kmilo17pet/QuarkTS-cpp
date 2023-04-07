#ifndef QOS_CPP_TYPES
#define QOS_CPP_TYPES

#include <cstddef>
#include <cstdint>
#include <cstdlib>

/**
* @brief A type to instantiate a integer-base variable. This type is
* compiler implementation-defined.
*/
using base_t = int;

/**
* @brief A type to instantiate a byte variable
*/
using byte_t = std::uint8_t;

/**
* @brief A type to instantiate a single-precision variable of 32-bits IEEE 754.
*/
using float32_t = float;  /*this is not always true in some compilers*/

/**
* @brief A type to instantiate an OS index variable. Can store the maximum
* size of a theoretically possible object of any type (including array).
* Should be used for array indexing and loop counting.
*/
using index_t = std::size_t;

/**
* @brief A type to instantiate a variable that hold the number of task cycles.
*/
using cycles_t = std::uint32_t;

/**
* @brief A type to instantiate a variable that hold the number of task iterations.
*/
using iteration_t = std::int32_t;

/**
* @brief A type to instantiate a variable to hold the priority value of a task.
*/
using priority_t = std::uint8_t;

#define     Q_UNUSED(arg)     (void)(arg)
#define     Q_NONE            /*EMPTY MACRO*/

#endif /*QOS_CPP_TYPES*/
