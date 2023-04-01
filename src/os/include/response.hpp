#ifndef QOS_RESPONSE
#define QOS_RESPONSE

#include "types.hpp"
#include "timer.hpp"

namespace qOS {

    enum responseStatus {
        MISSING = 0,
        SUCCESS,
        TIMEOUT,
    };

    class response {
        private:
            char *pattern2Match{ nullptr };
            timer timeout;
            std::size_t maxStrLength{ 0u };
            std::size_t patternLength{ 0u };
            volatile std::size_t matchedCount{ 0u };
            volatile bool responseReceived{ false };
            response( response const& ) = delete;
            void operator=( response const& ) = delete;
        public:
            response() = default;
            bool setup( char *xLocBuff, const std::size_t nMax ) noexcept;
            void reset( void ) noexcept;
            responseStatus received( const char *pattern, const std::size_t n, qOS::time_t t ) noexcept;
            inline responseStatus received( const char *pattern, const std::size_t n ) noexcept
            {
                return received( pattern, n, clock::IMMEDIATE );
            }
            bool isrHandler( const char rxChar ) noexcept;
            bool initialized( void ) const noexcept;
    };

}
#endif /*QOS_RESPONSE*/
