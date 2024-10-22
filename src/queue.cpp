#include "include/queue.hpp"
#include "include/critical.hpp"

using namespace qOS;

/*============================================================================*/
bool queue::setup( void *pData, const size_t size, const size_t count ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != pData ) && ( size > 0U ) && ( count > 0U ) ) {
        itemsCount = count;
        itemSize = size;
        /*cstat -CERT-EXP36-C_b*/
        head = static_cast<uint8_t*>( pData );
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
    itemsWaiting = 0U;
    writer = head;
    reader = head + ( ( itemsCount - 1U )*itemSize );
    /*cstat +CERT-INT30-C_a*/
    critical::exit();
}
/*============================================================================*/
bool queue::isEmpty( void ) const noexcept
{
    return ( 0U == itemsWaiting );
}
/*============================================================================*/
bool queue::isFull( void ) const noexcept
{
    return ( itemsWaiting == itemsCount );
}
/*============================================================================*/
size_t queue::count( void ) const noexcept
{
    return itemsWaiting;
}
/*============================================================================*/
size_t queue::itemsAvailable( void ) const noexcept
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
    size_t waiting;

    critical::enter();
    waiting = itemsWaiting;
    if ( waiting > 0U ) {
        moveReader();
        itemsWaiting = itemsWaiting - 1U; /* --itemsWaiting */
        retValue = true;
    }
    critical::exit();

    return retValue;
}
/*============================================================================*/
void queue::copyDataFromQueue( void * const dst ) noexcept
{
    moveReader();
    (void)memcpy( static_cast<void*>( dst ), static_cast<void*>( reader ), itemSize );
}
/*============================================================================*/
bool queue::receive( void *dst ) noexcept
{
    bool retValue = false;
    size_t waiting;

    critical::enter();
    waiting = itemsWaiting;
    if ( waiting > 0U ) {
        copyDataFromQueue( dst );
        itemsWaiting = itemsWaiting - 1U; /* --itemsWaiting */
        retValue = true;
    }
    critical::exit();


    return retValue;
}
/*============================================================================*/
void queue::copyDataToQueue( const void *itemToQueue, const queueSendMode xPosition ) noexcept
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
    itemsWaiting = itemsWaiting + 1U; /* ++itemsWaiting */
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
    uint8_t *retValue = nullptr;
    size_t waiting;

    critical::enter();
    waiting = itemsWaiting;
    if ( waiting > 0U ) {
        retValue = static_cast<uint8_t *>( reader + itemSize );
        if ( retValue >= tail ) {
            retValue = head;
        }
    }
    critical::exit();

    return static_cast<void*>( retValue );
}
/*============================================================================*/
bool queue::isInitialized( void ) const noexcept
{
    return ( nullptr != head );
}
/*============================================================================*/
size_t queue::getItemSize( void ) const noexcept
{
    return itemSize;
}
/*============================================================================*/
