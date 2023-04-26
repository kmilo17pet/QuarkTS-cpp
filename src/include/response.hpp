#ifndef QOS_RESPONSE
#define QOS_RESPONSE

#include "include/types.hpp"
#include "include/timer.hpp"

namespace qOS {

    /** @addtogroup qresponse Response handler
    * @brief API to simplify the handling of requested responses from terminal
    * interfaces.
    *  @{
    */

    /**
    * @brief A enum with the possible return status of a qOS::response object.
    */
    enum class responseStatus {
        MISSING = 0,
        SUCCESS,
        TIMEOUT,
    };

    /**
    * @brief A Response Handler object.
    */
    class response {
        private:
            char *pattern2Match{ nullptr };
            timer timeout;
            size_t maxStrLength{ 0u };
            size_t patternLength{ 0u };
            volatile size_t matchedCount{ 0u };
            volatile bool responseReceived{ false };
            response( response const& ) = delete;
            void operator=( response const& ) = delete;
        public:
            response() = default;
            virtual ~response() {}
            /**
            * @brief Initialize the instance of the response handler object
            * @param[in] xLocBuff A pointer to the memory block where the desired
            * response will remain.
            * @param[in] nMax The size of @a xLocBuff
            * @return On success returns @c true, otherwise returns @c false.
            */
            bool setup( char *xLocBuff, const size_t nMax ) noexcept;
            /**
            * @brief Reset the Response Handler
            */
            void reset( void ) noexcept;
            /**
            * @brief Non-Blocking response check
            * @param[in] pattern The data checked in the receiver ISR
            * @param[in] n The length of the data pointer by @a pattern
            * (if @a pattern is string, set @a n to 0 to auto-compute the length)
            * @param[in] t The timeout value given in milliseconds.
            * @return responseStatus::SUCCESS if there is a response acknowledge,
            * responseStatus::TIMEOUT if timeout expires otherwise returns 
            * responseStatus::MISSING
            */
            responseStatus received( const char *pattern, const size_t n, qOS::time_t t = clock::IMMEDIATE ) noexcept;
            /**
            * @brief ISR receiver for the response handler
            * @param[in] rxChar The byte-data from the receiver
            * @return True when the Response handler match the request from
            * response::received()
            */
            bool isrHandler( const char rxChar ) noexcept;
            /**
            * @brief Check if the response object is already initialized by 
            * using response::setup()
            * @return @c true if the response object is initialized, @c false if not.
            */
            bool isInitialized( void ) const noexcept;
    };

    /** @}*/
}

#endif /*QOS_RESPONSE*/
