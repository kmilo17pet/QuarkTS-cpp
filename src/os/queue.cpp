#include "queue.hpp"
#include "critical.hpp"
#include <cstring>


using namespace qOS;

/*============================================================================*/
queue::queue( void *pData, const std::size_t itemSize, const std::size_t itemsCount )
{
    (void)setup( pData, itemSize, itemsCount );
}
/*============================================================================*/
bool queue::setup( void *pData, const std::size_t itemSize, const std::size_t itemsCount )
{
    bool retValue = false;

    if ( ( nullptr != pData ) && ( itemSize > 0u ) && ( itemsCount > 0u ) ) {
        this->itemsCount = itemsCount; /* Initialise the queue members*/
        this->itemSize = itemSize;
        head = static_cast<std::uint8_t*>( pData );
        reset();
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
void queue::reset( void )
{
    critical::enter();
    tail = head + ( itemsCount*itemSize );
    itemsWaiting = 0u;
    writer = head;
    reader = head + ( ( itemsCount - 1u )*itemSize );
    critical::exit();
}
/*============================================================================*/
bool queue::isEmpty( void ) const
{
    return ( 0u == itemsWaiting ) ? true : false;
}
/*============================================================================*/
bool queue::isFull( void ) const
{
    return ( itemsWaiting == itemsCount ) ? true : false;
}
/*============================================================================*/
std::size_t queue::count( void ) const
{
    return itemsWaiting;
}
/*============================================================================*/
std::size_t queue::itemsAvailable( void ) const
{
    return itemsCount - itemsWaiting;
}
/*============================================================================*/
void queue::moveReader( void )
{
    reader += itemSize;
    if ( reader >= tail ) {
        reader = head;
    }
}
/*============================================================================*/
bool queue::removeFront( void )
{
    bool retValue = false;
    std::size_t waiting;

    critical::enter();
    waiting = itemsWaiting; /*to avoid side effects*/
    if ( waiting > 0u ) {
        moveReader();
        --itemsWaiting; /* remove the data. */
        retValue = true;
    }
    critical::exit();

    return retValue;
}
/*============================================================================*/
void queue::copyDataFromQueue( void * const dst )
{
    moveReader();
    (void)memcpy( static_cast<void*>( dst ), static_cast<void*>( reader ), itemSize );
}
/*============================================================================*/
bool queue::receive( void *dst )
{
    bool retValue = false;
    std::size_t waiting;

    critical::enter();
    waiting = itemsWaiting; /*to avoid side effects*/
    if ( waiting > 0u ) {
        /* items available, remove one of them. */
        copyDataFromQueue( dst );
        --itemsWaiting;
        retValue = true;
    }
    critical::exit();


    return retValue;
}
/*============================================================================*/
void queue::copyDataToQueue( const void *itemToQueue, const queueSendMode xPosition )
{
    if ( queueSendMode::TO_BACK == xPosition ) {
        (void)memcpy( static_cast<void*>( writer ), itemToQueue, itemSize );
        writer += itemSize;
        if ( writer >= tail ) {
            writer = head;
        }
    }
    else {
        (void)memcpy( static_cast<void*>( reader ), itemToQueue, itemSize );
        reader -= itemSize;
        if ( reader < head ) {
            reader = ( tail - itemSize );
        }
    }
    ++itemsWaiting;
}
/*============================================================================*/
bool queue::send( void *itemToQueue, const queueSendMode pos )
{
    bool retValue = false;

    if ( ( queueSendMode::TO_BACK == pos  ) || ( queueSendMode::TO_FRONT == pos ) ) {
        critical::enter();
        if ( itemsWaiting < itemsCount ) {
            copyDataToQueue( itemToQueue, pos );
            retValue = true;
        }
        critical::exit();
    }

    return retValue;
}
/*============================================================================*/
void* queue::peek( void ) const
{
    std::uint8_t *retValue = nullptr;
    std::size_t waiting;

    critical::enter();
    waiting = itemsWaiting; /*to avoid side effects*/
    if ( waiting > 0u ) {
        retValue = static_cast<std::uint8_t *>( reader + itemSize );
        if ( retValue >= tail ) {
            retValue = head;
        }
    }
    critical::exit();

    return static_cast<void*>( retValue );
}
/*============================================================================*/
bool queue::isReady( void ) const
{
    return ( nullptr != head );
}
/*============================================================================*/
std::size_t queue::getItemSize( void )
{
    return itemSize;
}
/*============================================================================*/
