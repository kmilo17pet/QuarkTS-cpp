#pragma once

#include "types.hpp"

namespace qOS {

    typedef enum {
        AT_FRONT = -1,
        AT_BACK = -2
    } listPosition;

    typedef struct _listCompareHandle_s {
        void *n1;                 /**< Points to the node that is currently being processed*/
        void *n2;                 /**< Points to the node that is currently being processed*/
    } _listCompareHandle_t;
    typedef _listCompareHandle_t* listCompareHandle_t;
   
    class node {
        protected:
            node *next = nullptr;
            node *prev = nullptr;
        public:
            void *container = nullptr;
            node();
            void init( void );
        friend class list;
        friend class listIterator;
    };

    typedef bool (*listCompareFcn_t)( listCompareHandle_t h );

    typedef enum {
        FORWARD,
        BACKWARD
    } listDirection;

    class listIterator;

    class list : protected node {
        protected:
            node *head = nullptr;
            node *tail = nullptr;
            node *iter = nullptr;
            listDirection iDir = FORWARD;
            size_t size = 0u;
            bool isMember( const void * const xNode ) const;
            void insertAtFront( node * const xNode );
            void insertAtBack( node * const xNode );
            node* removeFront( void );
            node* removeBack( void );
            node* getNodeAtIndex( const listPosition p ) const;
            void givenNodeSwapBoundaries( node *n1, node *n2 );
            void givenNodeSwapAdjacent( node *n1, node *n2 );
            void givenNodesUpdateOuterLinks( node *n1, node *n2 );
            list( list const& ) = delete;      /* not copyable*/
            void operator=( list const& ) = delete;  /* not assignable*/
        public:
            list();
            bool insert( void * const xNode, const listPosition p );
            bool remove( void * const xNode );
            void* remove( void * const xNode, const listPosition p );
            void* getFront( void ) const;
            void* getBack( void ) const;
            bool isEmpty( void ) const;
            size_t length( void ) const;
            bool sort( listCompareFcn_t f );
            bool swap( void* node1, void* node2 );
            bool move( list *src, const listPosition p );
            void clean( void );
            listIterator begin( void );
            listIterator end( void );
            listIterator from( void *offset );
        friend class listIterator;
    };

    class listIterator {
        protected:
            list *l;
            node *iter = nullptr;
            void *current = nullptr;
        public:
            listIterator() = delete;
            listIterator( list *xList ) : listIterator( xList, FORWARD, nullptr ) {}
            listIterator( list *xList, listDirection dir ) : listIterator( xList, dir, nullptr ) {}
            listIterator( list *xList, listDirection dir, void *nodeOffset );
            bool until( void );
            bool until( void* node );
            listIterator& operator++( int );
            listIterator& operator--( int );
            template <typename T>
            T get( void ) {
                return static_cast<T>( current );
            }
    };

}