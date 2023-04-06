#ifndef QOS_CPP_LIST
#define QOS_CPP_LIST

#include "types.hpp"

namespace qOS {

    enum listPosition : std::int32_t {
        AT_FRONT = -1,
        AT_BACK = -2
    };

    struct _listCompareHandle_s {
        void *n1;                 /**< Points to the node that is currently being processed*/
        void *n2;                 /**< Points to the node that is currently being processed*/
    };
    using listCompareHandle_t = struct _listCompareHandle_s*;
   
    class node {
        private:
            node *next = nullptr;
            node *prev = nullptr;
            void init( void ) noexcept;
            node( node const& ) = delete;
            void operator=( node const& ) = delete;
        public:
            void *container = nullptr;
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

    class list : protected node {
        private:
            node *head{ nullptr };
            node *tail{ nullptr };
            node *iter{ nullptr };
            listDirection iDir{ listDirection::FORWARD };
            std::size_t size{ 0u };
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
            list() noexcept ;
            bool insert( void * const xNode, const listPosition p = listPosition::AT_BACK ) noexcept;
            bool remove( void * const xNode ) noexcept;
            void* remove( void * const xNode, const listPosition p ) noexcept;
            void* getFront( void ) const noexcept;
            void* getBack( void ) const noexcept;
            bool isEmpty( void ) const noexcept;
            std::size_t length( void ) const noexcept;
            bool sort( listCompareFcn_t f ) noexcept;
            bool swap( void* node1, void* node2 ) noexcept;
            bool move( list& src, const listPosition p ) noexcept;
            void clean( void ) noexcept;
            listIterator begin( void ) noexcept;
            listIterator end( void ) noexcept;
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
