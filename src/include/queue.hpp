#ifndef QOS_CPP_QUEUE
#define QOS_CPP_QUEUE

#include "include/types.hpp"

namespace qOS {

    /**
    * @brief An enum that defines the modes in which a item should be inserted 
    * in to a queue.
    */
    enum class queueSendMode {
        TO_BACK,    /*< to indicate whether the item in the queue should be sent to the back. */
        TO_FRONT    /*< to indicate whether the item in the queue should be sent to the front. */
    };

    /**
    * @brief A Queue object
    * @details A queue is a linear data structure with simple operations based
    * on the FIFO (First In First Out) principle. It is capable to hold a finite
    * number of fixed-size data items. The maximum number of items that a queue
    * can hold is called its length. Both the length and the size of each data
    * item are set when the queue is created.
    *
    * In general, this kind of data structure is used to serialize data between
    * tasks, allowing some elasticity in time. In many cases, the queue is used
    * as a data buffer in interrupt service routines. This buffer will collect
    * the data so, at some later time, another task can fetch the data for
    * further processing. This use case is the single "task to task" buffering
    * case. There are also other applications for queues as serializing  many
    * data streams into one receiving streams (multiple tasks to a single task)
    * or vice-versa (single task to multiple tasks).
    *
    * The queue::setup() method configures the queue and initialize the instance.
    * The required RAM for the queue data should be provided by the application
    * writer and could be statically allocated at compile time or in run-time
    * using the memory management extension.
    */
    class queue {
        private:
            uint8_t *head{ nullptr };
            uint8_t *tail{ nullptr };
            uint8_t *writer{ nullptr };
            uint8_t *reader{ nullptr };
            volatile size_t itemsWaiting = 0u;
            size_t itemsCount = 0u;
            size_t itemSize = 0u;
            void moveReader( void ) noexcept;
            void copyDataFromQueue( void * const dst ) noexcept;
            void copyDataToQueue( const void *itemToQueue, const queueSendMode xPosition ) noexcept; 
            queue( queue const& ) = delete;
            void operator=( queue const& ) = delete;
        public:
            queue() = default;
            /**
            * @brief Configures a Queue. Here, the RAM used to hold the queue data
            * @a pData is statically allocated at compile time by the application writer.
            * @param[in] pData Data block or array of data.
            * @param[in] size The size, in bytes, of one single item in the queue.
            * @param[in] count The maximum number of items the queue can hold.
            * @return true on success, otherwise returns false.
            */
            bool setup( void *pData, const size_t size, const size_t count ) noexcept;
            /**
            * @brief Resets a queue to its original empty state.
            */
            void reset( void ) noexcept;
            /**
            * @brief Returns the empty status of the Queue
            * @return true if the Queue is empty, false if it is not.
            */
            bool isEmpty( void ) const noexcept;
            /**
            * @brief Returns the full status of the Queue
            * @return true if the Queue is full, false if it is not.
            */
            bool isFull( void ) const noexcept;
            /**
            * @brief Returns the number of items in the Queue
            * @return The number of elements in the queue
            */
            size_t count( void ) const noexcept;
            /**
            * @brief Returns the number of available slots to hold items inside 
            * the queue.
            * @return The number of available slots in the queue.
            */
            size_t itemsAvailable( void ) const noexcept;
            /**
            * @brief Remove the data located at the front of the Queue
            * @return true if data was removed from the Queue, otherwise returns
            * false
            */
            bool removeFront( void ) noexcept;
            /**
            * @brief Receive an item from a queue (and removes it). The item is 
            * received by copy so a buffer of adequate size must be provided.
            * The number of bytes copied into the buffer was defined when the 
            * queue was created.
            * @param[out] dst Pointer to the buffer into which the received item
            * will be copied.
            * @return true if data was retrieved from the Queue, otherwise returns
            * #false
            */
            bool receive( void *dst ) noexcept;
            /**
            * @brief Post an item to the the queue. The item is queued by copy,
            * not by reference
            * @param[in] itemToQueue A pointer to the item that is to be placed on the
            * queue. The size of the items the queue will hold was defined when the
            * queue was created, so this many bytes will be copied from @a itemToQueue
            * into the queue storage area.
            * @param[in] pos Can take the value queueSendMode::TO_BACK (default)
            * to place the item at the back of the queue, or queueSendMode::TO_FRONT
            * to place the item at the front of the queue (for high priority messages).
            * @return true on successful add, false if not added
            */
            bool send( void *itemToQueue, const queueSendMode pos = queueSendMode::TO_BACK ) noexcept;
            /**
            * @brief Looks at the data from the front of the Queue without removing it.
            * @return Pointer to the data, or @c nullptr if there is nothing in the queue.
            */
            void* peek( void ) const noexcept;
            /**
            * @brief Check if the queue is already initialized by using queue::setup()
            * @return true if the queue is initialized, false if not.
            */
            bool isInitialized( void ) const noexcept;
            size_t getItemSize( void ) const noexcept;
    };
}

#endif /*QOS_CPP_QUEUE*/
