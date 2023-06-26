#ifndef QOS_CPP_HELPER
#define QOS_CPP_HELPER

#include "include/types.hpp"

namespace qOS {

    /** @addtogroup qflm
    * @{
    */

    /**
    * @brief Helper Bit-manipulation interfaces.
    */
    namespace bits {
        /** @addtogroup qflm
        * @{
        */

        /*cstat -CERT-INT34-C_a -MISRAC++2008-5-0-10*/
        /**
        * @brief Uses the bitwise OR operator to set bits into @a dst
        * @param[in] dst The variable where the operation will be applied
        * @param[in] xBits Indicates which bits are to be set
        */
        template <typename T, typename W>
        inline void multipleSet( T &dst, W xBits )
        {
            dst |= static_cast<T>( xBits );
        }
        /**
        * @brief Uses the bitwise AND operator to clear bits into @a dst
        * @param[in] dst The variable where the operation will be applied
        * @param[in] xBits Indicates which bits are to be cleared
        */
        template <typename T, typename W>
        inline void multipleClear( T &dst, W xBits )
        {
            dst &= ~static_cast<T>( xBits );
        }
        /**
        * @brief Uses the bitwise OR operator to set a single bit into @a dst
        * @param[in] dst The variable where the operation will be applied
        * @param[in] xBit The bit-index
        */
        template <typename T>
        inline void singleSet( T &dst, const size_t xBit )
        {
            dst |= ( static_cast<T>( 1u ) << static_cast<T>( xBit ) );
        }
        /**
        * @brief Uses the bitwise AND operator to clear a single bit into @a dst
        * @param[in] dst The variable where the operation will be applied
        * @param[in] xBit The bit-index
        */
        template <typename T>
        inline void singleClear( T &dst, const size_t xBit )
        {
            dst &= ~( static_cast<T>( 1u ) << static_cast<T>( xBit ) );
        }
        /**
        * @brief Uses the bitwise AND operator to read the value of a single bit
        * from @a dst
        * @param[in] dst The variable where the operation will be applied
        * @param[in] xBit The bit-index
        * @return The value of the requested bit.
        */
        template <typename T>
        inline bool singleRead( T dst, const size_t xBit )
        {
            return ( static_cast<T>( 0u ) == ( dst & ( static_cast<T>( 1u ) << static_cast<T>( xBit ) ) ) ) ? false : true;
        }
        /**
        * @brief Uses the bitwise XOR operator to toggle the value of a single bit
        * from @a dst
        * @param[in] dst The variable where the operation will be applied
        * @param[in] xBit The bit-index
        */
        template <typename T>
        inline void singleToggle( T &dst, const size_t xBit )
        {
            dst ^= ( static_cast<T>( 1u ) << static_cast<T>( xBit ) );
        }
        /*cstat +CERT-INT34-C_a +MISRAC++2008-5-0-10*/
        /**
        * @brief Write the boolean @a value in a single bit of the @a dst variable
        * @param[in] dst The variable where the operation will be applied
        * @param[in] xBit The bit-index
        * @param[in] value The boolean value to write
        * @return none.
        */
        template <typename T>
        inline void singleWrite( T &dst, const size_t xBit, const bool value )
        {
            ( value ) ? bitSet( dst, xBit ) : bitClear( dst, xBit );
        }
        /**
        * @brief Read multiple bits by applying the mask given by @a xBits to
        * the @a dst variable
        * @param[in] reg The variable where the operation will be applied
        * @param[in] xBits The bit mask
        * @return The @a dst variable masked with @a xBits.
        */
        template <typename T, typename W>
        inline bool multipleGet( T reg, W xBits )
        {
            return ( 0u != ( reg & xBits) ) ? true : false;
        }
        /** @}*/
    }

    /*! @cond  */
    template <typename DT, typename ST>
    inline DT aligned_cast( ST& src )
    {
        DT dst;
        (void)memcpy( &dst, &src, sizeof(ST) );
        return dst;
    }
    /*! @endcond  */

    /*! @cond  */
    template <typename T>
    constexpr size_t arraySize( const T& ) noexcept
    {
        return 1;
    }
    /*! @endcond  */
    /**
     * @brief Calculates the size of an array.
     * This function takes a reference to an array and calculates its size.
     * For a one-dimensional array, the size is simply the number of elements.
     * For a multi-dimensional array, the size is the product of the sizes of 
     * all dimensions.
     * @warning This function only works for raw-arrays. It is not intended for 
     * boost or STL arrays and vectors
     * @tparam T The type of the array elements.
     * @tparam n The size of the first dimension (for a multi-dimensional array).
     * @param arr The array whose size to calculate.
     * @return The size of the array.
     */
    template <typename T, size_t n>
    constexpr size_t arraySize( const T (&arr)[n] ) noexcept
    {
        return n*arraySize(*arr);
    }
    /**
    * @brief Ensures that @a x is between the limits set by @a Min and @a Max.
    * @note If @a Min is greater than @a Max the result is undefined.
    * @param[in] x The value to clamp
    * @param[in] Min The minimum value allowed.
    * @param[in] Max The maximum value allowed.
    * @return The value of @a x clamped to the range between @a Min and @a Max.
    */
    template <typename T>
    inline T clip( T x, const T Min, const T Max )
    {
        return ( x < Min ) ? Min : ( ( x > Max ) ? Max : x );
    }
    /**
    * @brief Ensures that @a x  is bellow the limits set by @a Max.
    * @param[in] x The value to clamp
    * @param[in] Max The maximum value allowed.
    * @return The value of @a x clamped to the value of @a Max.
    */
    template <typename T>
    inline T clipUpper( T x, const T Max )
    {
        return ( x > Max ) ? Max : x ;
    }
    /**
    * @brief Ensures that @a x  is above the value set by @a Min.
    * @param[in] x The value to clamp
    * @param[in] Min The minimum value allowed.
    * @return The value of @a x clamped to the value of @a Min.
    */
    template <typename T>
    inline T clipLower( T x, const T Min )
    {
        return ( x < Min ) ? Min : x ;
    }
    /**
    * @brief Check if the value of @a x  is within the limits defined by
    * @a Low and @a High.
    * @param[in] x The value to check
    * @param[in] Low The minimum value allowed.
    * @param[in] High The maximum value allowed.
    * @return @c true if the @a x is between the defined range. Otherwise returns
    * @c false.
    */
    template <typename T>
    inline T isBetween( T x, const T Low, const T High )
    {
        return ( x >= Low ) && ( x <= High );
    }
    /**
    * @brief Read the high-nibble from @a x
    * @param[in] x The input variable
    * @return The value of the high nibble taken from @a x
    */
    inline uint8_t byteNibbleHigh( uint8_t x )
    {
        return static_cast<uint8_t>( x >> 4u );
    }
    /**
    * @brief Read the low-nibble from @a x
    * @param[in] x The input variable
    * @return The value of the low nibble taken from @a x
    */
    inline uint8_t byteNibbleLow( uint8_t x )
    {
        return static_cast<uint8_t>( x & 0x0Fu );
    }
    /**
    * @brief Merges two nibbles to form one byte.
    * @param[in] h The high nibble
    * @param[in] l The low nibble
    * @return The value of the resulting byte
    */
    inline uint8_t byteMergeNibbles( uint8_t h, uint8_t l )
    {
        return static_cast<uint8_t>( ( h << 4u ) | ( 0x0Fu & l ) );
    }
    /**
    * @brief Read the high-byte from @a x
    * @param[in] x The input variable
    * @return The value of the high byte taken from @a x
    */
    inline uint16_t wordByteHigh( uint16_t x )
    {
        return static_cast<uint16_t>( x >> 8u );
    }
    /**
    * @brief Read the low-byte from @a x
    * @param[in] x The input variable
    * @return The value of the low byte taken from @a x
    */
    inline uint16_t wordByteLow( uint16_t x )
    {
        return static_cast<uint16_t>( x & 0x00FFu );
    }
    /**
    * @brief Merges two bytes to form one Word.
    * @param[in] h The high byte
    * @param[in] l The low byte
    * @return The value of the resulting word
    */
    inline uint16_t wordMergeBytes( uint16_t h, uint16_t l )
    {
        return static_cast<uint16_t>( ( h << 8u ) | ( 0x00FFu & l ) );
    }
    /**
    * @brief Read the high-word from @a x
    * @param[in] x The input variable
    * @return The value of the high word taken from @a x
    */
    inline uint32_t dWordWordHigh( uint32_t x )
    {
        return static_cast<uint32_t>( x >> 16u );
    }
    /**
    * @brief Read the low-word from @a x
    * @param[in] x The input variable
    * @return The value of the low word taken from @a x
    */
    inline uint32_t dWordWordLow( uint32_t x )
    {
        return static_cast<uint32_t>( x & 0xFFFFu );
    }
    /**
    * @brief Merges two words to form one DWord.
    * @param[in] h The high byte
    * @param[in] l The low byte
    * @return The value of the resulting word
    */
    inline uint32_t dwordMergeWords( uint32_t h, uint32_t l )
    {
        return static_cast<uint32_t>( ( h << 16u ) | ( 0xFFFFu & l ) );
    }

    /** @}*/
}

#endif /*QOS_CPP_HELPER*/
