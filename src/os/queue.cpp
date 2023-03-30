#include "queue.hpp"
#include "critical.hpp"
#include <cstring>


using namespace qOS;

/*============================================================================*/
queue::queue( void *pData, const std::size_t size, const std::size_t count ) noexcept
{
    (void)setup( pData, size, count );
}
/*============================================================================*/
bool queue::setup( void *pData, const std::size_t size, const std::size_t count ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != pData ) && ( size > 0u ) && ( count > 0u ) ) {
        this->itemsCount = count; /* Initialise the queue members*/
        this->itemSize = size;
        /*cstat -CERT-EXP36-C_b*/
        head = static_cast<std::uint8_t*>( pData );
        /*cstat +CERT-EXP36-C_b*/
        reset();
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
void queue::reset( void ) noexcept
{
    critical::enter();
    /*cstat -CERT-INT30-C_a*/
    tail = head + ( itemsCount*itemSize );
    itemsWaiting = 0u;
    writer = head;
    reader = head + ( ( itemsCount - 1u )*itemSize );
    /*cstat +CERT-INT30-C_a*/
    critical::exit();
}
/*============================================================================*/
bool queue::isEmpty( void ) const noexcept
{
    return ( 0u == itemsWaiting ) ? true : false;
}
/*============================================================================*/
bool queue::isFull( void ) const noexcept
{
    return ( itemsWaiting == itemsCount ) ? true : false;
}
/*============================================================================*/
std::size_t queue::count( void ) const noexcept
{
    return itemsWaiting;
}
/*============================================================================*/
std::size_t queue::itemsAvailable( void ) const noexcept
{
    return itemsCount - itemsWaiting;
}
/*============================================================================*/
void queue::moveReader( void ) noexcept
{
    reader += itemSize;
    if ( reader >= tail ) {
        reader = head;
    }
}
/*============================================================================*/
bool queue::removeFront( void ) noexcept
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
void queue::copyDataFromQueue( void * const dst ) noexcept
{
    moveReader();
    (void)std::memcpy( static_cast<void*>( dst ), static_cast<void*>( reader ), itemSize );
}
/*============================================================================*/
bool queue::receive( void *dst ) noexcept
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
void queue::copyDataToQueue( const void *itemToQueue, const queueSendMode xPosition ) noexcept
{
    if ( queueSendMode::TO_BACK == xPosition ) {
        (void)std::memcpy( static_cast<void*>( writer ), itemToQueue, itemSize );
        writer += itemSize;
        if ( writer >= tail ) {
            writer = head;
        }
    }
    else {
        (void)std::memcpy( static_cast<void*>( reader ), itemToQueue, itemSize );
        reader -= itemSize;
        if ( reader < head ) {
            reader = ( tail - itemSize );
        }
    }
    ++itemsWaiting;
}
/*============================================================================*/
bool queue::send( void *itemToQueue, const queueSendMode pos ) noexcept
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
void* queue::peek( void ) const noexcept
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
bool queue::isReady( void ) const noexcept
{
    return ( nullptr != head );
}
/*============================================================================*/
std::size_t queue::getItemSize( void ) noexcept
{
    return itemSize;
}
/*============================================================================*/
