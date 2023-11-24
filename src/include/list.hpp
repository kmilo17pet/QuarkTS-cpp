#ifndef QOS_CPP_LIST
#define QOS_CPP_LIST

#include "include/types.hpp"

namespace qOS {

    /** 
    * @addtogroup qlists
    * @brief The provided list implementation uses a generic doubly-linked 
    * approach in which each node, apart from storing its data, has two link 
    * pointers. The first link points to the previous node in the list and the 
    * second link, points to the next node in the list. The first node of the 
    * list has its previous link pointing to @c nullptr, similarly, the last node
    * of the list has its next node pointing to @c nullptr.
    * 
    * The list data-structure, referenced through an object of type qList_t 
    * also has a head and a tail pointer, to allow fast operations on boundary 
    * nodes.
    * 
    * <center>
    * @htmlonly
    * <!DOCTYPE html>
    * <html>
    * <head>
    * <title>qlist</title>
    * <meta charset="utf-8"/>
    * </head>
    * <body><div class="mxgraph" style="max-width:100%;border:1px solid transparent;" data-mxgraph="{&quot;highlight&quot;:&quot;#0000ff&quot;,&quot;nav&quot;:true,&quot;zoom&quot;:1.2,&quot;resize&quot;:true,&quot;toolbar&quot;:&quot;zoom layers tags lightbox&quot;,&quot;edit&quot;:&quot;_blank&quot;,&quot;xml&quot;:&quot;&lt;mxfile host=\&quot;app.diagrams.net\&quot; modified=\&quot;2022-10-19T20:37:00.387Z\&quot; agent=\&quot;5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/106.0.0.0 Safari/537.36\&quot; etag=\&quot;KSbS3wh_9uupMrEyc6ED\&quot; version=\&quot;20.4.1\&quot; type=\&quot;google\&quot;&gt;&lt;diagram id=\&quot;mmlpRbxcPcDZs_50ANYI\&quot; name=\&quot;Página-1\&quot;&gt;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&lt;/diagram&gt;&lt;/mxfile&gt;&quot;}"></div>
    * <script type="text/javascript" src="https://viewer.diagrams.net/js/viewer-static.min.js"></script>
    * </body>
    * </html>
    * @endhtmlonly
    * <em>Doubly-linked list implementation</em>
    * </center>
    * 
    * Nodes should be an user-defined class inherited fromt he node class 
    * 
    * @code{.c}
    * class mynode : public node {
    *     int a;
    *     int b;
    *     float y;
    * };
    * @endcode
    * 
    *  @{
    */

    /**
    * @brief An enum with the possible options to specify a target position for a list.
    */
    enum listPosition : int32_t {
        AT_FRONT = -1,
        AT_BACK = -2
    };

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
            /**
            * @brief Get a pointer to the list in which this node is contained.
            * @return A pointer to the list container.
            */
            inline list* getContainer( void ) noexcept
            {
                return container;
            }
            node() : next(nullptr), prev(nullptr), container(nullptr) {}
            virtual ~node() {}
            
        friend class list;
        friend class listIterator;
    };

    /**
    * @brief Pointer to a function used by the list::sort() method to compare
    * nodes of a list.
    *
    * Example :
    * @code{.c}
    * bool myNode_CompareFcn( const void *n1, const void *n2 ) {
    *     mydata_t *node1 = (mydata_t *)n1;
    *     mydata_t *node2 = (mydata_t *)n2;
    *     return ( node1->x > node2->x );
    * }
    * @endcode
    * @param[in] h The handler object containing the objects being compared.
    * @return  @c true value indicates that element pointed by @a node1 goes
    * after the element pointed to by @a node2
    */
    using listCompareFcn_t = bool (*)( const void *, const void * );

    /**
    * @brief An enum with the possible options to transverse a list.
    */
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
            size_t size{ 0U };
            bool isMember( const void * const xNode ) const noexcept;
            void insertAtFront( node * const xNode ) noexcept;
            void insertAtBack( node * const xNode ) noexcept;
            node* removeFront( void ) noexcept;
            node* removeBack( void ) noexcept;
            node* getNodeAtIndex( const listPosition p ) const noexcept;
            void givenNodeSwapBoundaries( node *n1, node *n2 ) noexcept;
            static void givenNodeSwapAdjacent( node *n1, node *n2 ) noexcept;
            static void givenNodesUpdateOuterLinks( node *n1, node *n2 ) noexcept;
            list( list const& ) = delete;
            void operator=( list const& ) = delete;
        public:
            list() noexcept;
            /**
            * @brief Insert an item into the list.
            * @param[in] xNode A pointer to the node to be inserted
            * @param[in] p The position where the node will be inserted. Could be
            * listPosition::AT_FRONT, listPosition::AT_BACK or any other index 
            * number where the node will be inserted after.
            * @return @c true if the item was successfully added to the list, otherwise
            * returns @c false
            */
            bool insert( void * const xNode, const listPosition p = listPosition::AT_BACK ) noexcept;
            /**
            * @brief If the node is member of a list, the node will be removed from it.
            * @param[in] xNode A pointer to the node.
            * @return @c true on Success. @c false if removal can't be performed.
            */
            bool remove( void * const xNode ) noexcept;
            /**
            * @brief Remove an item from the list.
            * @param[in] p The position of the node that will be removed. Could be
            * listPosition::AT_FRONT, listPosition::AT_BACK or any other index 
            * number.
            * @param[in] xNode A pointer to the node to be deleted (to ignore 
            * pass @c nullptr ).
            * @return A pointer to the removed node. @c nullptr if removal 
            * can not be performed.
            */
            void* remove( const listPosition p, void * const xNode = nullptr ) noexcept;
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
            * @return @c true if the list is empty, @c false if it is not.
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
            * where a @c true value indicates that element pointed by @a node1 goes
            * after the element pointed to by @a node2.
            * @return @c true if at least one reordering is performed over the list.
            */
            bool sort( listCompareFcn_t f ) noexcept;
            /**
            * @brief Swap two nodes that belongs to the list by changing its own
            * links.
            * @note The list containing nodes will be updated if any node is part
            * of the boundaries.
            * @param[in] node1 Pointer to the first node.
            * @param[in] node2 Pointer to the second node.
            * @return @c true if the swap operation is performed. Otherwise returns
            * @c false.
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
            * @return @c true if the move operation is performed successfully, 
            * otherwise returns @c false
            */
            bool move( list& src, const listPosition p = listPosition::AT_BACK ) noexcept;
            /**
            * @brief Clean up the entire list leaving it empty
            */
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

    /**
    * @brief A list iterator
    */
    class listIterator {
        private:
            list *l{ nullptr };
            node *iter{ nullptr };
            void *current{ nullptr };
        public:
            listIterator() = delete;
            /**
            * @brief Instantiate a list iterator for the given list.
            * @param[in] xList The list 
            * @param[in] dir The direction in which the iterator will traverse 
            * the list
            * @param[in] nodeOffset A pointer to the node offset in the list
            */
            explicit listIterator( list& xList, listDirection dir = listDirection::FORWARD, void *nodeOffset = nullptr ) noexcept;
            /**
            * @brief Check until current iterator reach one of its ends
            * @return @c true if the iterator has reach on of its ends.
            */
            bool until( void ) const noexcept;
            /**
            * @brief Check until current iterator reach the given node
            * @param[in] node A pointer to the node you want to reach
            * @return @c true if the iterator has reach the given node.
            */
            bool until( void* node ) const noexcept;
            /**
            * @brief Move the iterator forward
            */
            listIterator& operator++( int ) noexcept;
            /**
            * @brief Move the iterator backward
            */
            listIterator& operator--( int ) noexcept;
            /**
            * @brief Gets the node that the iterator is currently pointing to.
            * @return A pointer to the node currently being pointed.
            */
            template <typename T>
            inline T get( void ) noexcept
            {
                /*cstat -CERT-EXP36-C_b*/
                return static_cast<T>( current );
                /*cstat +CERT-EXP36-C_b*/
            }
    };
    /** @}*/
}

#endif /*QOS_CPP_LIST*/
