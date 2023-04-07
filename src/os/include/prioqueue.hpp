#ifndef QOS_CPP_PRIO_QUEUE
#define QOS_CPP_PRIO_QUEUE

#include "types.hpp"
#include "task.hpp"

using namespace qOS;

struct _queueStack_s {
    task *Task{nullptr};         /*< A pointer to the task. */
    void *queueData{nullptr};    /*< The data to queue. */
};
using queueStack_t = struct _queueStack_s;

class prioQueue {
    private:
        volatile base_t index = -1;
        queueStack_t *stack{ nullptr };
        std::size_t size{ 0u };
        prioQueue( prioQueue const& ) = delete;
        void operator=( prioQueue const& ) = delete;
    public:
        void *data = nullptr;
        prioQueue() = delete;
        prioQueue( queueStack_t *area, const std::size_t pq_size ) noexcept;
        std::size_t count( void ) noexcept;
        task* get( void ) noexcept;
        bool isTaskInside( task &Task ) const noexcept;
        bool insert( task &Task, void *pData ) noexcept;
        void clearIndex( index_t indexToClear ) noexcept;
        void cleanUp( const task &Task ) noexcept;
};

#endif /*QOS_CPP_PRIO_QUEUE*/
