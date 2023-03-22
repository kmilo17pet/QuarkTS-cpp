#pragma once

#include "types.hpp"
#include "task.hpp"

using namespace qOS;

typedef struct _qQueueStack_s {
    task *Task;         /*< A pointer to the task. */
    void *queueData;    /*< The data to queue. */
} queueStack_t;

class prioQueue {
    protected:
        volatile base_t index = -1;
        queueStack_t *stack;
        size_t size;
        prioQueue( prioQueue const& ) = delete;      /* not copyable*/
        void operator=( prioQueue const& ) = delete;  /* not assignable*/
    public:
        void *data = nullptr;
        prioQueue() = delete;
        prioQueue( queueStack_t *area, const size_t size );
        size_t count( void );
        task* get( void );
        bool isTaskInside( task *Task );
        bool insert( task *Task, void *pData );
        void clearIndex( index_t indexToClear );
        void cleanUp( const task * Task );
};