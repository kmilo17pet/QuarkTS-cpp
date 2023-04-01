#ifndef QOS_CPP_TYPES
#define QOS_CPP_TYPES

#include <cstddef>
#include <cstdint>
#include <cstdlib>

using base_t = int;
using byte_t = std::uint8_t;
using float32_t = float;  /*this is not always true in some compilers*/
using index_t = std::size_t ;
using cycles_t = std::uint32_t ;
using iteration_t = std::int32_t;
using priority_t = std::uint8_t;

#define     Q_UNUSED(arg)     (void)(arg)
#define     Q_NONE            /*EMPTY MACRO*/

#endif /*QOS_CPP_TYPES*/
