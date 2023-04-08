#ifndef QOS_CPP_LIST
#define QOS_CPP_LIST

#include "types.hpp"

namespace qOS {

    enum listPosition : int32_t {
        AT_FRONT = -1,
        AT_BACK = -2
    };

    /**
    * @brief Handle of the list::sort() method that is passed as an argument to
    * the compare function.
    * @note The user must verify the input nodes to return a boolean value
    * that determines the order in which the nodes should be ordered
    */
    struct _listCompareHandle_s {
        void *n1;                 /**< Points to the node that is currently being processed*/
        void *n2;                 /**< Points to the node that is currently being processed*/
    };
    using listCompareHandle_t = struct _listCompareHandle_s*;

    class list;
    /**
    * @brief A list-node object (Used internally)
    */
    class node {
        private:
            node *next{ nullptr };
            node *prev{ nullptr };
            list *container{ nullptr };
            void init( void ) noexcept;
            node( node const& ) = delete;
            void operator=( node const& ) = delete;
        public:
            inline list* getContainer( void ) noexcept
            {
                return container;
            }
            node() : next(nullptr), prev(nullptr), container(nullptr) {}
            
        friend class list;
        friend class listIterator;
    };

    using listCompareFcn_t = bool (*)( listCompareHandle_t );

    enum class listDirection {
        FORWARD,
        BACKWARD
    };

    class listIterator;

    /**
    * @brief A list object (Generic double-linked)
    */
    class list : protected node {
        private:
            node *head{ nullptr };
            node *tail{ nullptr };
            node *iter{ nullptr };
            listDirection iDir{ listDirection::FORWARD };
            size_t size{ 0u };
            bool isMember( const void * const xNode ) const noexcept;
            void insertAtFront( node * const xNode ) noexcept;
            void insertAtBack( node * const xNode ) noexcept;
            node* removeFront( void ) noexcept;
            node* removeBack( void ) noexcept;
            node* getNodeAtIndex( const listPosition p ) const noexcept;
            void givenNodeSwapBoundaries( node *n1, node *n2 ) noexcept;
            void givenNodeSwapAdjacent( node *n1, node *n2 ) noexcept;
            void givenNodesUpdateOuterLinks( node *n1, node *n2 ) noexcept;
            list( list const& ) = delete;
            void operator=( list const& ) = delete;
        public:
            list() noexcept;
            /**
            * @brief Insert an item into the list.
            * @param[in] node A pointer to the node to be inserted
            * @param[in] p The position where the node will be inserted. Could be
            * listPosition::AT_FRONT, listPosition::AT_BACK or any other index 
            * number where the node will be inserted after.
            * @return true if the item was successfully added to the list, otherwise
            * returns false
            */
            bool insert( void * const xNode, const listPosition p = listPosition::AT_BACK ) noexcept;
            /**
            * @brief If the node is member of a list, the node will be removed from it.
            * @param[in] node A pointer to the node.
            * @return true on Success. false if removal can't be performed.
            */
            bool remove( void * const xNode ) noexcept;
            /**
            * @brief Remove an item from the list.
            * @param[in] node A pointer to the node to be deleted (to ignore 
            * pass @c nullptr ). 
            * @param[in] p The position of the node that will be removed. Could be
            * listPosition::AT_FRONT, listPosition::AT_BACK or any other index 
            * number.
            * @return A pointer to the removed node. @c nullptr if removal 
            * can not be performed.
            */
            void* remove( void * const xNode, const listPosition p ) noexcept;
            /**
            * @brief Get a pointer to the front item of the list
            * @return A pointer to the front node. @c nullptr if the list is empty
            */
            void* getFront( void ) const noexcept;
            /**
            * @brief Get a pointer to the back item of the list
            * @return A pointer to the back node. @c nullptr if the list is empty
            */
            void* getBack( void ) const noexcept;
            /**
            * @brief Check if the list is empty.
            * @return true if the list is empty, false if it is not.
            */
            bool isEmpty( void ) const noexcept;
            /**
            * @brief Get the number of items inside the list.
            * @return The number of items of the list.
            */
            size_t length( void ) const noexcept;
            /**
            * @brief Sort the double linked list using the @a f function to
            * determine the order. The sorting algorithm used by this function compares
            * pairs of adjacent nodes by calling the specified @a f function
            * with pointers to them as arguments. The sort is performed only
            * modifying node's links without data swapping, improving performance if
            * nodes have a large storage.
            * @note The function modifies the content of the list by reordering its
            * elements as defined by @a f.
            * @param[in] f Pointer to a function that compares two nodes.
            * This function is called repeatedly by list::sort() to compare two nodes.
            * It shall follow the following prototype:
            * @code bool CompareFcn( listCompareHandle_t h ) @endcode
            *
            * The function defines the order of the elements by returning a Boolean data,
            * where a #qTrue value indicates that element pointed by @a node1 goes
            * after the element pointed to by @a node2.
            * @return true if at least one reordering is performed over the list.
            */
            bool sort( listCompareFcn_t f ) noexcept;
            /**
            * @brief Swap two nodes that belongs to the list by changing its own
            * links.
            * @note The list containing nodes will be updated if any node is part
            * of the boundaries.
            * @param[in] node1 Pointer to the first node.
            * @param[in] node2 Pointer to the second node.
            * @return true if the swap operation is performed. Otherwise returns
            * false.
            */
            bool swap( void* node1, void* node2 ) noexcept;
            /**
            * @brief Moves(or merge) the entire list @a src to the given list.
            * After the move operation, this function leaves empty the list 
            * given by @a src.
            * @param[in] src Source list to be moved.
            * @param[in] p The position where @a src list will be inserted. 
            * Could be listPosition::AT_FRONT, listPosition::AT_BACK or any 
            * other index number.
            * @return true if the move operation is performed successfully, 
            * otherwise returns false
            */
            bool move( list& src, const listPosition p ) noexcept;
            void clean( void ) noexcept;
            /**
            * @brief Returns an iterator pointing to the first element in the 
            * list container.
            * @return An iterator to the beginning of the sequence container.
            */
            listIterator begin( void ) noexcept;
            /**
            * @brief Returns an iterator pointing to the last element in the 
            * list container.
            * @return An iterator to the latest item of the sequence container.
            */
            listIterator end( void ) noexcept;
            /**
            * @brief Returns an iterator pointing to the element given by 
            * @a offset in the list container.
            * @return An iterator to the @a offset of the sequence container.
            */
            listIterator from( void *offset ) noexcept;
        friend class listIterator;
    };

    class listIterator {
        private:
            list *l{ nullptr };
            node *iter{ nullptr };
            void *current{ nullptr };
        public:
            listIterator() = delete;
            listIterator( list& xList, listDirection dir = listDirection::FORWARD, void *nodeOffset = nullptr )  noexcept;
            bool until( void ) noexcept;
            bool until( void* node ) noexcept;
            listIterator& operator++( int ) noexcept;
            listIterator& operator--( int ) noexcept;
            template <typename T>
            inline T get( void ) noexcept
            {
                /*cstat -CERT-EXP36-C_b*/
                return static_cast<T>( current );
                /*cstat +CERT-EXP36-C_b*/
            }
    };

}

#endif /*QOS_CPP_LIST*/
