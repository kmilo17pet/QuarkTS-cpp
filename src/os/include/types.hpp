#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>

typedef int base_t;
typedef uint8_t byte_t;
typedef float float32_t;  /*this is not always true in some compilers*/
typedef double float64_t; /*this is not always true in some compilers*/
typedef size_t index_t; /*better portability*/
typedef uint32_t cycles_t;
typedef int32_t iteration_t;
typedef uint8_t priority_t;

#define qLink                   ( true )
#define qUnLink                 ( false )