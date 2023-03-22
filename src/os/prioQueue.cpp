#include "prioqueue.hpp"
#include "critical.hpp"

using namespace qOS;

/*============================================================================*/
prioQueue::prioQueue( queueStack_t *area, const size_t size )
{
    critical::enter();
    stack = area;
    this->size = size;
    for ( size_t i = 0u ; i < size ; ++i ) {
        stack[ i ].Task = nullptr;  /*set the priority queue as empty*/
    }
    index = -1;
     critical::exit();
}
/*============================================================================*/
size_t prioQueue::count( void )
{
    size_t retValue = (size_t)0u;

    if ( index >= 0 ) {
        retValue = static_cast<size_t>( index ) + 1u;
    }

    return retValue;
}
/*============================================================================*/
task* prioQueue::get( void )
{
    task *xTask = nullptr;

    if ( index >= 0 ) { /*queue has elements*/
        priority_t maxPriority;
        index_t indexTaskToExtract = 0u;

        critical::enter();
        maxPriority = stack[ 0 ].Task->getPriority();
        /*walk through the queue to find the task with the highest priority*/
        /*loop until all items are checked or if the tail is reached*/
        for ( index_t i = 1u ; ( i < size ) && ( nullptr != stack[ i ].Task ) ; ++i ) {
            priority_t iPriorityValue = stack[ i ].Task->getPriority();
            /*check if the queued task has the max priority value*/
            if ( iPriorityValue > maxPriority ) {
                maxPriority = iPriorityValue; /*Reassign the max value*/
                indexTaskToExtract = i;  /*save the index*/
            }
        }
        /*get the data from the queue*/
        data = this->stack[ indexTaskToExtract ].queueData;
        /*assign the task to the output*/
        xTask = stack[ indexTaskToExtract ].Task;
        clearIndex( indexTaskToExtract );
        critical::exit();
    }

    return xTask;
}
/*============================================================================*/
bool prioQueue::isTaskInside( task *Task )
{
    bool retValue = false;
    base_t currentQueueIndex = index + 1;
    /*check first if the queue has items inside*/
    if ( currentQueueIndex > 0 ) {
        critical::enter();
        /*loop the queue slots to check if the Task is inside*/
        for ( base_t i = 0 ; i < currentQueueIndex ; ++i ) {
            if ( Task == stack[ i ].Task ) {
                retValue = true;
                break;
            }
        }
        critical::exit();
    }

    return retValue;
}
/*============================================================================*/
bool prioQueue::insert( task *Task, void *pData )
{
    bool retValue = false;
    const base_t queueMaxIndex = size - 1;

    /*check if data can be queued*/
    if ( ( nullptr != Task )  && ( index < queueMaxIndex ) ) {
        queueStack_t tmp{ Task, pData };
        /*insert task and the corresponding eventData to the queue*/
        stack[ ++index ] = tmp;
        retValue = true;
    }
    return retValue;
}
/*============================================================================*/
void prioQueue::clearIndex( index_t indexToClear )
{
    base_t queueIndex = static_cast<base_t>( index ); /*to avoid side effects*/
    
    stack[ indexToClear ].Task = nullptr; /*set the position in the queue as empty*/
    for ( index_t j = indexToClear ; static_cast<base_t>( j ) < queueIndex ; ++j ) {
        stack[ j ] = stack[ j + 1u ]; /*shift the remaining items of the queue*/
    }
    --index; /*decrease the index*/
}
/*============================================================================*/
void prioQueue::cleanUp( const task * Task )
{
    for ( index_t i = 1u ; i < size ; ++i ) {
        if ( stack[ i ].Task == Task ) {
            clearIndex( i );
        }
    }
}
/*============================================================================*/