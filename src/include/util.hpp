#ifndef QOS_CPP_UTIL
#define QOS_CPP_UTIL

#include "include/types.hpp"


namespace qOS {

    /**
    * @brief Utilities.
    */
    namespace util {

        /** @addtogroup qioutils I/O Utils
        * @brief API for input/output utilities and safe string interfaces.
        *  @{
        */

        /**
        * @brief Pointer to function that write-out a single character
        * @note  User should use bare-metal code to implement this function.
        * Example :
        * @code{.c}
        * void BSP_PutChar( void *sp, const char c ) {
        *       (void)sp;
        *       HAL_UARTWriteByte( c );
        * }
        * @endcode
        * @param[in] sp The user storage pointer.
        * @param[in] c The character to write out.
        * @return none.
        */
        using putChar_t = void (*)( void* , const char );

        /**
        * @brief Pointer to function that perform a single character I/O operation
        * @note  User should use bare-metal code to implement this function.
        * Example 1: Input operation
        * @code{.c}
        * char BSP_GetChar( void *sp, const char in ) {
        *       (void)sp;
        *       (void)in;
        *       return HAL_UARTReceiveByte( );
        * }
        * @endcode
        * Example 2: Ouput operation
        * @code{.c}
        * char BSP_GetChar( void *sp, char in ) {
        *       (void)sp;
        *       HAL_UARTPutByte( in );
        *       return 0u;
        * }
        * @endcode
        * @param[in] sp The user storage pointer.
        * @param[in] in The byte to write out.
        * @return If an input operation is performed, this function should return
        * the byte read from the input.
        */
        using ioFcn_t = char (*)( void *, const char );

        /**
        * @brief Returns a pointer to the first occurrence of character in the C
        * string @a s.
        * The terminating null-character is considered part of the C string.
        * Therefore, it can also be located in order to retrieve a pointer to the
        * end of a string.
        * @param[in] s Pointer to the null-terminated byte string to be examined
        * @param[in] c Character to be located. It is passed as its int promotion,
        * but it is internally converted back to char for the comparison.
        * @param[in] maxlen Maximum number of characters to examine
        * @return A pointer to the first occurrence of character in @a s.
        * If the character is not found, the function returns a null pointer.
        */
        char* strchr( const char *s, int c, size_t maxlen ) noexcept;
        /**
        * @brief Returns the length of the given null-terminated byte string, that
        * is, the number of characters in a character array whose first element is
        * pointed to by @a str up to and not including the first null character.
        * The function returns zero if @a str is a null pointer and returns
        * @a maxlen if the null character was not found in the first @a maxlen bytes
        * of @a str.
        * @param[in] s Pointer to the null-terminated byte string to be examined
        * @param[in] maxlen Maximum number of characters to examine
        * @return The length of the null-terminated byte string @a str on success,
        * zero if @a str is a null pointer, @a maxlen if the null character was 
        * not found.
        */
        size_t strlen( const char* s, size_t maxlen ) noexcept;
        /**
        * @brief Copies up to @a (maxlen - 1) characters from the null-terminated
        * string @a src to
        * @a dst, null-terminating the result.
        * @param[out] dst The destination string
        * @param[in] src The source string
        * @param[in] maxlen Maximum number of characters to copy
        * @return The length of @a src
        */
        size_t strcpy( char * dst, const char * src, size_t maxlen ) noexcept;
        /**
        * @brief appends no more than @a maxlen−len(dst)−1 characters pointed to by 
        * @a src into the array pointed to by @a dst and always terminates the 
        * result with a null character if @a maxlen is greater than zero. Both 
        * the strings @a dst and @a src must be terminated with a null character on
        * entry the function and a byte for the terminating null should be included 
        * in @a maxlen. 
        * @note The behavior of this function is undefined if copying takes
        * place between objects that overlap.
        * @param[out] dst The destination string
        * @param[in] src The source string
        * @param[in] maxlen Maximum number of characters to copy
        * @return returns the number of characters it tried to copy, which is the 
        * sum of the lengths of the strings @a dst and @a src or @a n, whichever 
        * is smaller.
        */
        size_t strcat( char *dst, const char *src, size_t maxlen ) noexcept;
        /**
        * @brief Invert the endianess for n bytes of the specified memory location.
        * @param[in,out] pData A pointer to block of data
        * @param[in] n The number of bytes to swap
        * @return @c true on success, otherwise returns @c false.
        */
        bool swapBytes( void *pData, const size_t n ) noexcept;
        /**
        * @brief Check the system endianess
        * @return @c true if Little-Endian, otherwise returns @c false.
        */
        bool checkEndianness( void ) noexcept;
        /**
        * @brief API interface to write a string through @a fcn
        * @param[in] fcn The basic output byte function
        * @param[in] pStorage The storage pointer passed to @a fcn
        * @param[in] s The string to be written
        * @param[in] aip Auto-Increment the storage-pointer
        * @return @c true on success, otherwise returns @c false.
        */
        bool outputString( util::putChar_t fcn, const char *s, void* pStorage = nullptr, bool aip = false ) noexcept;
        /**
        * @brief API interface to write data in HEX notation through @a fcn
        * @param[in] fcn The basic output byte function
        * @param[in] pStorage The storage pointer passed to @a fcn
        * @param[in] pData A pointer to the block of data
        * @param[in] n The number of bytes to print out
        * @param[in] eol @c true to enable the EOL(End-Of-Line) termination.
        * @return @c true on success, otherwise returns @c false.
        */
        bool printXData( util::putChar_t fcn, void *pData, size_t n, bool eol = true, void* pStorage = nullptr ) noexcept;
        /**
        * @brief API interface to write n RAW data through @a fcn
        * @param[in] fcn The basic output byte function
        * @param[in] pData A pointer to the block of data
        * @param[in] n The number of bytes that will be transferred to the output
        * @param[in] pStorage The storage pointer passed to @a fcn
        * @param[in] aip Auto-increment the storage-pointer
        * @return @c true on success, otherwise returns @c false.
        */
        bool outputRAW( const ioFcn_t fcn, void *pData, const size_t n, void* pStorage = nullptr, bool aip = false ) noexcept;
        /**
        * @brief API interface to get n RAW data through @a fcn
        * @param[in] fcn The basic input byte function
        * @param[out] pData A pointer to the block where the read data will be saved
        * @param[in] n The number of bytes to get
        * @param[in] pStorage The storage pointer passed to @a fcn
        * @param[in] aip Auto-increment the storage-pointer
        * @return @c true on success, otherwise returns @c false.
        */
        bool inputRAW( const ioFcn_t fcn, void *pData, const size_t n, void* pStorage = nullptr, bool aip = false ) noexcept;
        /**
        * @brief Converts the input string consisting of hexadecimal digits into an
        * unsigned integer value. The input parameter should consist exclusively of
        * hexadecimal digits, with optional whitespaces. The string will be
        * processed one character at a time, until the function reaches a character
        * which it doesn't recognize (including a null character).
        * @param[in] s The hex string to be converted.
        * @return The numeric value as @c uint32_t.
        */
        uint32_t hexStringToUnsigned( const char *s ) noexcept;
        /**
        * @brief Parses the C string @a s, interpreting its content as a floating
        * point number and returns its value as a float( @c float32_t). The function
        * first discards as many whitespace characters (as in isspace) as necessary
        * until the first non-whitespace character is found. Then, starting from
        * this character, takes as many characters as possible that are valid
        * following a syntax resembling that of floating point literals, and
        * interprets them as a numerical value. The rest of the string after the
        * last valid character is ignored and has no effect on the behavior of this
        * function.
        * @param[in] s The string beginning with the representation of a
        * floating-point number.
        * @return On success, the function returns the converted floating point
        * number as a float( @c qFloat32_t ) value.
        * If no valid conversion could be performed, the function returns zero (0.0f).
        * If the converted value would be out of the range of representable values
        * by a float( @c qFloat32_t ), it causes undefined behavior
        */
        float64_t stringToFloat( const char *s ) noexcept;
        /**
        * @brief Converts a float value to a formatted string.
        * @param[in] num Value to be converted to a string.
        * @param[out] str  Array in memory where to store the resulting
        * null-terminated string.
        * @param[in] precision Desired number of significant fractional digits in
        * the string. (default = 10u )
        * @return A pointer to the resulting null-terminated string, same as
        * parameter @a str.
        */
        char* floatToString( float64_t num, char *str, uint8_t precision = 10u ) noexcept;
        /**
        * @brief Parses the C-string @a s interpreting its content as an integral
        * number, which is returned as a value of type int. The function first
        * discards as many whitespace characters (as in @c isspace) as necessary until
        * the first non-whitespace character is found. Then, starting from this
        * character, takes an optional initial plus or minus sign followed by as
        * many base-10 digits as possible, and interprets them as a numerical value.
        * The string can contain additional characters after those that form the
        * integral number, which are ignored and have no effect on the behavior of
        * this function. If the first sequence of non-whitespace characters in @a s
        * is not a valid integral number, or if no such sequence exists because
        * either @a s is empty or it contains only whitespace characters, no
        * conversion is performed and zero is returned.
        * @param[in] s The string beginning with the representation of a integer
        * number.
        * @return On success, the function returns the converted integral number as
        * an int value.
        * If the converted value would be out of the range of representable values
        * by an @c int, it causes undefined behavior.
        */
        int stringToInt( const char *s ) noexcept;
    /**
        * @brief Converts an unsigned value to a null-terminated string using the
        * specified base and stores the result in the array given by @a str
        * parameter. The argument @a str should be an array long enough to contain
        * any possible value: @c "sizeof(int)*8+1" for radix=2, i.e. 17 bytes in
        * 16-bits platforms and 33 in 32-bits platforms.
        * @param[in] num Value to be converted to a string.
        * @param[out] str  Array in memory where to store the resulting
        * null-terminated string.
        * @param[in] base Numerical base used to represent the value as a string,
        * between 2 and 36, where 10 means decimal base, 16 hexadecimal, 8 octal,
        * and 2 binary.
        * @return A pointer to the resulting null-terminated string, same as
        * parameter @a str.
        */
        char* unsignedToString( unsigned_t num, char* str, uint8_t base = 10u ) noexcept;
        /**
        * @brief Converts an integer value to a null-terminated string using the
        * specified base and stores the result in the array given by @a str
        * parameter. If base is 10 and value is negative, the resulting string is
        * preceded with a minus sign (-). With any other base, value is always
        * considered unsigned.
        *
        * The argument @a str should be an array long enough to contain any possible
        * value: (sizeof(int)*8+1) for radix=2, i.e. 17 bytes in 16-bits platforms
        * and 33 in 32-bits platforms.
        * @param[in] num Value to be converted to a string.
        * @param[out] str  Array in memory where to store the resulting
        * null-terminated string.
        * @param[in] base Numerical base used to represent the value as a string,
        * between 2 and 36, where 10 means decimal base, 16 hexadecimal, 8 octal,
        * and 2 binary.
        * @return A pointer to the resulting null-terminated string, same as
        * parameter @a str.
        */
        char* integerToString( signed_t num, char* str, uint8_t base = 10u ) noexcept;
        /**
        * @brief Converts a boolean value to a null-terminated string. Input is
        * considered true with any value different to zero (0).
        * The argument @a str should be an array long enough to contain the output
        * @param[in] num Value to be converted to a string.
        * @param[out] str  Array in memory where to store the resulting
        * null-terminated string.
        * @return A pointer to the resulting null-terminated string, same as
        * parameter @a str.
        */
        char* boolToString( const bool num, char *str ) noexcept;

        /** @}*/
    }
    
}

#endif /*QOS_CPP_UTIL*/
