#include "include/list.hpp"

using namespace qOS;

/*============================================================================*/
void node::init( void ) noexcept
{
    next = nullptr;
    prev = nullptr;
    container = nullptr;
}
/*============================================================================*/
list::list() noexcept
{
    clean();
}
/*============================================================================*/
bool list::isMember( const void * const xNode ) const noexcept
{
    bool retValue = false;
    
    if ( nullptr != xNode ) {
        /*cstat -CERT-EXP36-C_b*/
        const node * const Node = static_cast<const node*>( xNode );
        
        if ( this == Node->container ) {
            retValue = true;
        }
        /*cstat +CERT-EXP36-C_b*/
    }
    
    return retValue;
}
/*============================================================================*/
void list::insertAtFront( node * const xNode ) noexcept
{
    xNode->next = this->head;
    head->prev = xNode;
    head = xNode;
}
/*============================================================================*/
void list::insertAtBack( node * const xNode ) noexcept
{
    tail->next = xNode;
    xNode->prev = tail;
    tail = xNode;
}
/*============================================================================*/
node* list::removeFront( void ) noexcept
{
    node * const removed = head;

    head = removed->next;
    if ( nullptr == head ) {
        tail = head;
    }
    else {
        head->prev = nullptr;
    }

    return removed;
}
/*============================================================================*/
node* list::removeBack( void ) noexcept
{
    node * const removed = tail;

    tail = removed->prev;
    if ( nullptr == tail ) {
        head = tail;
    }
    else {
        tail->next = nullptr;
    }

    return removed;
}
/*============================================================================*/
node* list::getNodeAtIndex( const listPosition p ) const noexcept
{
    node *iNode;
    base_t iPos = 0;
    /*cstat -MISRAC++2008-0-1-2_b*/
    for ( iNode = head ; ( iPos < static_cast<base_t>( p )  ) && ( nullptr != iNode->next ) ; iNode = iNode->next ) {
        ++iPos;
    }
    /*cstat +MISRAC++2008-0-1-2_b*/
    return iNode;
}
/*============================================================================*/
bool list::insert( void * const xNode, const listPosition p ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != xNode ) && ( p >= listPosition::AT_BACK ) ) {
        if ( false == isMember( xNode ) ) {
            /*cstat -CERT-EXP36-C_b*/
            node * const newNode = static_cast<node*>( xNode );
            /*cstat +CERT-EXP36-C_b*/
            newNode->init();
            
            retValue = true;
            if ( nullptr == head ){
                head = newNode;
                tail = newNode;
            }
            else if ( listPosition::AT_FRONT == p ) {
                insertAtFront( newNode );
            }
            /*cstat -MISRAC++2008-0-1-2_a*/
            else if ( listPosition::AT_BACK == p ) {
                insertAtBack( newNode );
            }
            /*cstat +MISRAC++2008-0-1-2_a*/
            else {
                node *iNode;
                iNode = getNodeAtIndex( p );
                newNode->next = iNode->next;  /* NEW -> (i+1)NODE */
                newNode->prev = iNode;        /* iNODE <- NEW */
                iNode->next->prev = newNode;  /* NEW <- (i+1)NODE */
                iNode->next = newNode;        /* iNODE -> NEW */
            }
            ++size;
            newNode->container = this;
        }
    }

    return retValue;
}
/*============================================================================*/
bool list::remove( void * const xNode ) noexcept
{
    bool retValue = false;
    
    if ( nullptr != xNode ) {
        /*cstat -CERT-EXP36-C_b*/
        node * const toRemove = static_cast<node*>( xNode );
        /*cstat +CERT-EXP36-C_b*/
        
        if ( this == toRemove->container ) {
            if ( toRemove == head ) {
                (void)removeFront();
            }
            else if ( toRemove == tail ) {
                (void)removeBack();
            }
            else {
                toRemove->prev->next = toRemove->next;
                if ( nullptr != toRemove->next ) {
                    toRemove->next->prev = toRemove->prev;
                }
            }
            --size;
            toRemove->container = nullptr;
            retValue = true;
        }
    }
    
    return retValue;
}
/*============================================================================*/
void* list::remove( void * const xNode, const listPosition p ) noexcept
{
    node *removed = nullptr;

    if ( ( nullptr != head ) && ( p >= listPosition::AT_BACK ) ) {
        if ( isMember( xNode ) ) {
            if ( remove( xNode ) ) {
                /*cstat -CERT-EXP36-C_b*/
                removed = static_cast<node*>( xNode );
                /*cstat +CERT-EXP36-C_b*/
            }
        }
        else {
            if ( listPosition::AT_FRONT == p ) {
                removed = removeFront();
            }
            /*cstat -MISRAC++2008-0-1-2_a*/
            else if ( listPosition::AT_BACK  == p ) {
                removed = removeBack();
            }
            /*cstat +MISRAC++2008-0-1-2_a*/
            else {
                node *iNode;
                const base_t lastIndex = ( static_cast<base_t>( p ) - 1 );

                iNode = getNodeAtIndex( static_cast<listPosition>( lastIndex ) );
                removed = iNode->next; /* <-> (inode0) <-> inode1 <-> inode2 */
                iNode->next = removed->next;
                if ( nullptr != removed->next ) {
                    iNode->next->prev = iNode;
                }
            }
            removed->container = nullptr;
            --size;
        }
    }

    return removed;
}
/*============================================================================*/
void* list::getFront( void ) const noexcept
{
    return static_cast<void*>( head );
}
/*============================================================================*/
void* list::getBack( void ) const noexcept
{
    return static_cast<void*>( tail );
}
/*============================================================================*/
bool list::isEmpty( void ) const noexcept
{
    return ( nullptr == head ) ? true : false;
}
/*============================================================================*/
size_t list::length( void ) const noexcept
{
    return size;
}
/*============================================================================*/
/*cstat -MISRAC++2008-7-1-2*/
bool list::sort( listCompareFcn_t f ) noexcept
{
/*cstat +MISRAC++2008-7-1-2*/
    bool retValue = false;

    if ( nullptr != f ) {
        const size_t count = size;

        if ( count >= 2u ) {
            node *current = nullptr, *before, *after;
            bool xRetCmp;

            for ( size_t i = 1u ; i < count ; ++i ) {
                const size_t n = count - i - 1u;
                current = head;
                for ( size_t j = 0u; j <= n; ++j ) {
                    xRetCmp = f( current, current->next );
                    if ( true == xRetCmp ) {
                        before = current->prev;
                        after = current->next;

                        if ( nullptr != before ) {
                            before->next = after;
                        }
                        else {
                            head = after;
                        }
                        current->next = after->next;
                        current->prev = after;

                        if ( nullptr != after->next ) {
                            after->next->prev = current;
                        }

                        after->next = current;
                        after->prev = before;
                        retValue = true;
                    }
                    else {
                        current = current->next;
                    }
                }
            }
            while ( nullptr != current->next ) {
                current = current->next;
            }
            tail = current;
        }
    }

    return retValue;
}
/*============================================================================*/
bool list::swap( void* node1, void* node2 ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != node1 ) && ( nullptr != node2 ) && ( node1 != node2 ) ) {
        /*cstat -CERT-EXP36-C_b*/
        node *n1 = static_cast<node*>( node1 );
        node *n2 = static_cast<node*>( node2 );
        /*cstat +CERT-EXP36-C_b*/
        if ( ( this == n1->container ) && ( n1->container == n2->container ) ) {
            /*nodes are part of the same list*/
            node *tmp1, *tmp2;

            if ( n2->next == n1 ) {
                /*cstat -CERT-EXP36-C_b*/
                n1 = static_cast<node*>( node2 );
                n2 = static_cast<node*>( node1 );
                /*cstat +CERT-EXP36-C_b*/
            }
            tmp1 = n1->prev;
            tmp2 = n2->next;
            givenNodeSwapBoundaries( n1, n2 );
            givenNodeSwapAdjacent( n1, n2 );
            n2->prev = tmp1;
            n1->next = tmp2;
            givenNodesUpdateOuterLinks( n1, n2 );
            retValue = true;
        }
    }

    return retValue;
}
/*============================================================================*/
void list::givenNodeSwapBoundaries( node *n1, node *n2 ) noexcept
{
    if ( head == n1 ) {
        head = n2;
    }
    else if ( head == n2 ) {
        head = n1;
    }
    else {
        /*nothing to do here*/
    }
    if ( tail == n1 ) {
        tail = n2;
    }
    else if ( tail == n2 ) {
        tail = n1;
    }
    else {
        /*nothing to do here*/
    }
}
/*============================================================================*/
void list::givenNodeSwapAdjacent( node *n1, node *n2 ) noexcept
{
    if ( ( ( n1->next == n2 ) && ( n2->prev == n1 ) ) || ( ( n1->prev == n2 ) && ( n2->next == n1 ) ) ) {
        n1->prev = n1->next;
        n2->next = n2->prev;
    }
    else {
        n1->prev = n2->prev;
        n2->next = n1->next;
    }
}
/*============================================================================*/
void list::givenNodesUpdateOuterLinks( node *n1, node *n2 ) noexcept
{
    if ( nullptr != n1->prev ) {
        n1->prev->next = n1;
    }
    if ( nullptr != n1->next ) {
        n1->next->prev = n1;
    }
    if ( nullptr != n2->prev ) {
        n2->prev->next = n2;
    }
    if ( nullptr != n2->next ) {
        n2->next->prev = n2;
    }
}
/*============================================================================*/
bool list::move( list& src, const listPosition p ) noexcept
{
    bool retValue = false;

    if ( ( nullptr != src.head) && ( p >= listPosition::AT_BACK ) ) {
        node *iNode;
        /* cppcheck-suppress postfixOperator */
        for ( listIterator i = src.begin() ; i.until() ; i++ ) {
            iNode = i.get<node*>();
            iNode->container = this;
        }
        if ( nullptr == head ) { /*destination is empty*/
            head = src.head;
            tail = src.tail;
        }
        else if ( AT_FRONT == p ) {
            src.tail->next = head;
            head->prev = src.tail;
            head = src.head;
        }
        /*cstat -MISRAC++2008-0-1-2_a*/
        else if ( AT_BACK == p ) {
            tail->next = src.head;
            src.head->prev = tail;
            tail = src.tail;
        }
        /*cstat +MISRAC++2008-0-1-2_a*/
        else {
            iNode = getNodeAtIndex( p );
            src.tail->next = iNode->next;
            src.head->prev = iNode;
            iNode->next = src.head;
        }
        size += src.size;
        src.clean();
        retValue = true;
    }

    return retValue;
}
/*============================================================================*/
void list::clean( void ) noexcept
{
    head = nullptr;
    tail = nullptr;
    size = 0u;
}
/*============================================================================*/
listIterator list::begin( void ) noexcept
{
    listIterator it( *this, listDirection::FORWARD, nullptr );
    return it;
}
/*============================================================================*/
listIterator list::end( void ) noexcept
{
    listIterator it( *this, listDirection::BACKWARD, nullptr );
    return it;
}
/*============================================================================*/
listIterator list::from( void *offset ) noexcept
{
    listIterator it( *this, listDirection::FORWARD, offset );
    return it;
}
/*============================================================================*/
listIterator::listIterator( list& xList, listDirection dir, void *nodeOffset ) noexcept
{
    node *ret;
    /*cstat -CERT-EXP36-C_b*/
    node * const offset = static_cast<node*>( nodeOffset );
    /*cstat +CERT-EXP36-C_b*/
    l = &xList;
    if ( listDirection::FORWARD == dir ) {
        ret = ( xList.isMember( nodeOffset ) ) ? offset : xList.head;
        iter = ( nullptr != ret ) ? ret->next : nullptr;
    }
    else {
        ret = ( xList.isMember( nodeOffset ) ) ? offset : xList.tail;
        iter = ( nullptr != ret ) ? ret->prev : nullptr;
    }
    current = static_cast<void*>( ret );
}
/*============================================================================*/
bool listIterator::until( void ) const noexcept
{
    return ( nullptr != current );
}
/*============================================================================*/
bool listIterator::until( void* node ) const noexcept
{
    return ( l->isMember( node ) ) && ( node != current );
}
/*============================================================================*/
listIterator& listIterator::operator++( int ) noexcept
{
    node * const ret = iter;
    iter = ( nullptr != iter ) ? iter->next : nullptr;
    current = static_cast<void*>( ret );
    return *this;
}
/*============================================================================*/
listIterator& listIterator::operator--( int ) noexcept
{
    node * const ret = iter;
    iter = ( nullptr != iter ) ? iter->prev : nullptr;
    current = static_cast<void*>( ret );
    return *this;
}
/*============================================================================*/
