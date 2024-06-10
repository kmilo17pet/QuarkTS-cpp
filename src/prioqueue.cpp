#include "include/prioqueue.hpp"
#include "include/critical.hpp"

using namespace qOS;

/*============================================================================*/
prioQueue::prioQueue( pq::queueStack_t *area, const size_t pq_size ) noexcept
{
    critical::enter();
    stack = area;
    size = pq_size;
    for ( size_t i = 0U ; i < size ; ++i ) {
        stack[ i ].pTask = nullptr;
    }
    index = -1;
    critical::exit();
}
/*============================================================================*/
size_t prioQueue::count( void ) const noexcept
{
    size_t retValue = 0U;

    if ( hasElements() ) {
        retValue = static_cast<size_t>( index ) + 1U;
    }

    return retValue;
}
/*============================================================================*/
task* prioQueue::get( void ) noexcept
{
    task *xTask = nullptr;

    if ( hasElements() ) {
        priority_t maxPriority;
        index_t indexTaskToExtract = 0U;

        critical::enter();
        maxPriority = stack[ 0 ].pTask->getPriority();
        for ( index_t i = 1U ; ( i < size ) && ( nullptr != stack[ i ].pTask ) ; ++i ) {
            const priority_t iPriorityValue = stack[ i ].pTask->getPriority();
            if ( iPriorityValue > maxPriority ) {
                maxPriority = iPriorityValue;
                indexTaskToExtract = i;
            }
        }
        data = stack[ indexTaskToExtract ].qData;
        xTask = stack[ indexTaskToExtract ].pTask;
        clearIndex( indexTaskToExtract );
        critical::exit();
    }

    return xTask;
}
/*============================================================================*/
bool prioQueue::isTaskInside( const task &Task ) const noexcept
{
    bool retValue = false;
    const base_t currentQueueIndex = index + 1;

    if ( currentQueueIndex > 0 ) {
        critical::enter();
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

    if ( index < queueMaxIndex ) {
        pq::queueStack_t tmp;

        tmp.pTask = &Task;
        tmp.qData = pData;
        index = index + 1; /* ++index  */
        stack[ index ] = tmp; /*stack[ ++index ] = tmp;*/
        retValue = true;
    }
    return retValue;
}
/*cstat +MISRAC++2008-7-1-2*/
/*============================================================================*/
void prioQueue::clearIndex( const index_t indexToClear ) noexcept
{
    const base_t queueIndex = static_cast<base_t>( index );

    stack[ indexToClear ].pTask = nullptr;
    for ( index_t j = indexToClear ; static_cast<base_t>( j ) < queueIndex ; ++j ) {
        stack[ j ] = stack[ j + 1U ];
    }
    index = index - 1;  /* --index; */
}
/*============================================================================*/
void prioQueue::cleanUp( const task &Task ) noexcept
{
    for ( index_t i = 1U ; i < size ; ++i ) {
        if ( stack[ i ].pTask == &Task ) {
            clearIndex( i );
        }
    }
}
/*============================================================================*/
