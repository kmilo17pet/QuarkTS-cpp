#include "prioqueue.hpp"
#include "critical.hpp"

using namespace qOS;

/*============================================================================*/
prioQueue::prioQueue( pq::queueStack_t *area, const size_t pq_size ) noexcept
{
    critical::enter();
    stack = area;
    size = pq_size;
    for ( size_t i = 0u ; i < size ; ++i ) {
        stack[ i ].pTask = nullptr;  /*set the priority queue as empty*/
    }
    index = -1;
    critical::exit();
}
/*============================================================================*/
size_t prioQueue::count( void ) noexcept
{
    size_t retValue = 0u;

    if ( hasElements() ) {
        retValue = static_cast<size_t>( index ) + 1u;
    }

    return retValue;
}
/*============================================================================*/
task* prioQueue::get( void ) noexcept
{
    task *xTask = nullptr;

    if ( hasElements() ) { /*queue has elements*/
        priority_t maxPriority;
        index_t indexTaskToExtract = 0u;

        critical::enter();
        maxPriority = stack[ 0 ].pTask->getPriority();
        /*walk through the queue to find the task with the highest priority*/
        /*loop until all items are checked or if the tail is reached*/
        for ( index_t i = 1u ; ( i < size ) && ( nullptr != stack[ i ].pTask ) ; ++i ) {
            const priority_t iPriorityValue = stack[ i ].pTask->getPriority();
            /*check if the queued task has the max priority value*/
            if ( iPriorityValue > maxPriority ) {
                maxPriority = iPriorityValue; /*Reassign the max value*/
                indexTaskToExtract = i;  /*save the index*/
            }
        }
        /*get the data from the queue*/
        data = stack[ indexTaskToExtract ].qData;
        /*assign the task to the output*/
        xTask = stack[ indexTaskToExtract ].pTask;
        clearIndex( indexTaskToExtract );
        critical::exit();
    }

    return xTask;
}
/*============================================================================*/
bool prioQueue::isTaskInside( task &Task ) const noexcept
{
    bool retValue = false;
    const base_t currentQueueIndex = index + 1;
    /*check first if the queue has items inside*/
    if ( currentQueueIndex > 0 ) {
        critical::enter();
        /*loop the queue slots to check if the Task is inside*/
        for ( base_t i = 0 ; i < currentQueueIndex ; ++i ) {
            if ( &Task == stack[ i ].pTask ) {
                retValue = true;
                break;
            }
        }
        critical::exit();
    }

    return retValue;
}
/*============================================================================*/
/*cstat -MISRAC++2008-7-1-2*/
bool prioQueue::insert( task &Task, void *pData ) noexcept
{
    bool retValue = false;
    const base_t queueMaxIndex = static_cast<base_t>( size ) - 1;

    /*check if data can be queued*/
    if ( index < queueMaxIndex ) {
        pq::queueStack_t tmp;
        
        tmp.pTask = &Task;
        tmp.qData = pData;
        stack[ ++index ] = tmp; /*insert task and eventData to the queue*/
        retValue = true;
    }
    return retValue;
}
/*cstat +MISRAC++2008-7-1-2*/
/*============================================================================*/
void prioQueue::clearIndex( index_t indexToClear ) noexcept
{
    const base_t queueIndex = static_cast<base_t>( index ); /*to avoid side effects*/
    
    stack[ indexToClear ].pTask = nullptr; /*set the position in the queue as empty*/
    for ( index_t j = indexToClear ; static_cast<base_t>( j ) < queueIndex ; ++j ) {
        stack[ j ] = stack[ j + 1u ]; /*shift the remaining items of the queue*/
    }
    --index; /*decrease the index*/
}
/*============================================================================*/
void prioQueue::cleanUp( const task &Task ) noexcept
{
    for ( index_t i = 1u ; i < size ; ++i ) {
        if ( stack[ i ].pTask == &Task ) {
            clearIndex( i );
        }
    }
}
/*============================================================================*/
