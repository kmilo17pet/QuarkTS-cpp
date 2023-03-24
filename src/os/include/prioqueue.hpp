#ifndef QOS_CPP_PRIOQUEUE
#define QOS_CPP_PRIOQUEUE

#include "types.hpp"
#include "task.hpp"

using namespace qOS;

struct _queueStack_s {
    task *Task;         /*< A pointer to the task. */
    void *queueData;    /*< The data to queue. */
};
using queueStack_t = struct _queueStack_s;

class prioQueue {
    protected:
        volatile base_t index = -1;
        queueStack_t *stack;
        std::size_t size;
        prioQueue( prioQueue const& ) = delete;      /* not copyable*/
        void operator=( prioQueue const& ) = delete;  /* not assignable*/
    public:
        void *data = nullptr;
        prioQueue() = delete;
        prioQueue( queueStack_t *area, const std::size_t size );
        std::size_t count( void );
        task* get( void );
        bool isTaskInside( task &Task );
        bool insert( task &Task, void *pData );
        void clearIndex( index_t indexToClear );
        void cleanUp( const task &Task );
};

#endif /*QOS_CPP_PRIOQUEUE*/