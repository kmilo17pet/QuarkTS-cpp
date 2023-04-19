#ifndef QOS_CPP_PRIO_QUEUE
#define QOS_CPP_PRIO_QUEUE

#include "include/types.hpp"
#include "include/task.hpp"

/*! @cond */

namespace qOS {

    namespace pq {
        struct _queueStack_s {
            task *pTask{nullptr};
            void *qData{nullptr};
        };
        using queueStack_t = struct _queueStack_s;
    }

    class prioQueue {
        private:
            volatile base_t index{ -1 };
            void *data{ nullptr };
            pq::queueStack_t *stack{ nullptr };
            size_t size{ 0u };
            void clearIndex( index_t indexToClear ) noexcept;
            prioQueue( prioQueue const& ) = delete;
            void operator=( prioQueue const& ) = delete;
        protected:
            prioQueue() = delete;
            ~prioQueue() {}
            prioQueue( pq::queueStack_t *area, const size_t pq_size ) noexcept;
            size_t count( void ) noexcept;
            task* get( void ) noexcept;
            bool isTaskInside( task &Task ) const noexcept;
            bool insert( task &Task, void *pData ) noexcept;
            void cleanUp( const task &Task ) noexcept;
            inline bool hasElements( void ) noexcept
            {
                return ( index >= 0 );
            }
        friend class core; /*only core can use this class*/
    };

}

/*! @endcond */

#endif /*QOS_CPP_PRIO_QUEUE*/
