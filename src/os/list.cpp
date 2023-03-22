#include "list.hpp"

using namespace qOS;

/*============================================================================*/
node::node()
{
    this->init();
}
/*============================================================================*/
void node::init( void ) 
{
    this->next = nullptr;
    this->prev = nullptr;
    this->container = nullptr;
}
/*============================================================================*/
list::list()
{
    clean();
}
/*============================================================================*/
bool list::isMember( const void * const xNode ) const
{
    bool retValue = false;
    
    if ( nullptr != xNode ) {
        const node *Node = static_cast< const node* >( xNode );
        if ( this == static_cast<const list * const>( Node->container ) ) {
            retValue = true;
        }
    }
    
    return retValue;
}
/*============================================================================*/
void list::insertAtFront( node * const xNode )
{
    xNode->next = this->head;
    this->head->prev = xNode;
    this->head = xNode;
}
/*============================================================================*/
void list::insertAtBack( node * const xNode )
{
    this->tail->next = xNode;
    xNode->prev = this->tail;
    this->tail = xNode;
}
/*============================================================================*/
node* list::removeFront( void )
{
    node *removed = this->head;

    this->head = removed->next;
    if ( nullptr == this->head ) {
        this->tail = this->head;
    }
    else {
        this->head->prev = nullptr;
    }

    return removed;
}
/*============================================================================*/
node* list::removeBack( void )
{
    node *removed = this->tail;

    this->tail = removed->prev;
    if ( nullptr == this->tail ) {
        this->head = this->tail;
    }
    else {
        this->tail->next = nullptr;
    }

    return removed;
}
/*============================================================================*/
node* list::getNodeAtIndex( const listPosition p ) const
{
    node *iNode;
    base_t iPos = 0;
    
    for ( iNode = this->head ; ( iPos < static_cast<base_t>( p )  ) && ( nullptr != iNode->next ) ; iNode = iNode->next ) {
        ++iPos;
    }

    return iNode;
}
/*============================================================================*/
bool list::insert( void * const xNode, const listPosition p )
{
    bool retValue = false;

    if ( ( nullptr != xNode ) && ( p >= listPosition::AT_BACK ) ) {
        if ( false == this->isMember( xNode ) ) {
            node *newNode = static_cast<node*>( xNode );
            newNode->init();
            
            retValue = true;
            if ( nullptr == this->head ){
                this->head = newNode;
                this->tail = newNode;
            }
            else if ( listPosition::AT_FRONT == p ) {
                this->insertAtFront( newNode );
            }
            else if ( listPosition::AT_BACK == p ) {
                this->insertAtBack( newNode );
            }
            else {
                node *iNode;
                iNode = this->getNodeAtIndex( p );
                newNode->next = iNode->next;  /* NEW -> (i+1)NODE */
                newNode->prev = iNode;        /* iNODE <- NEW */
                iNode->next->prev = newNode;  /* NEW <- (i+1)NODE */
                iNode->next = newNode;        /* iNODE -> NEW */
            }
            ++this->size;
            newNode->container = this;
        }
    }

    return retValue;
}
/*============================================================================*/
bool list::remove( void * const xNode )
{
    bool retValue = false;
    
    if ( nullptr != xNode ) {
        node *toRemove = static_cast<node*>( xNode );
        
        if ( nullptr != toRemove->container ) {
            list *l = static_cast<list*>( toRemove->container );
            
            if ( toRemove == this->head ) {
                (void)this->removeFront();
            }
            else if ( toRemove == this->tail ) {
                (void)this->removeBack();
            }
            else {
                toRemove->prev->next = toRemove->next;
                if ( nullptr != toRemove->next ) {
                    toRemove->next->prev = toRemove->prev;
                }
            }
            --this->size;
            toRemove->container = nullptr;
            retValue = true;
        }
    }
    
    return retValue;
}
/*============================================================================*/
void* list::remove( void * const xNode, const listPosition p )
{
    node *removed = nullptr;

    if ( ( nullptr != this->head ) && ( p >= listPosition::AT_BACK ) ) {
        if ( this->isMember( xNode ) ) {
            if ( this->remove( xNode ) ) {
                removed = static_cast<node*>( xNode ); 
            }
        }
        else {
            if ( listPosition::AT_FRONT == p ) {
                removed = this->removeFront();
            }
            else if ( listPosition::AT_BACK  == p ) {
                removed = this->removeBack();
            }
            else {
                node *iNode;
                base_t lastIndex = ( static_cast<base_t>( p ) - 1 );

                iNode = this->getNodeAtIndex( static_cast<listPosition>( lastIndex ) );
                removed = iNode->next; /* <-> (inode0) <-> inode1 <-> inode2 */
                iNode->next = removed->next;
                if ( nullptr != removed->next ) {
                    iNode->next->prev = iNode;
                }
            }
            removed->container = nullptr;
            --this->size;
        }
    }

    return removed;
}
/*============================================================================*/
void* list::getFront( void ) const
{
    return static_cast<void*>( this->head );
}
/*============================================================================*/
void* list::getBack( void ) const
{
    return static_cast<void*>( this->tail );
}
/*============================================================================*/
bool list::isEmpty( void ) const
{
    return ( nullptr == this->head ) ? true : false;
}
/*============================================================================*/
size_t list::length( void ) const
{
    return this->size;
}
/*============================================================================*/
bool list::sort( listCompareFcn_t f )
{
    bool retValue = false;

    if ( nullptr != f ) {
        size_t count = this->size;

        if ( count >= 2u ) {
            node *current = nullptr, *before, *after;
            _listCompareHandle_t xHandle;
            bool xRetCmp;

            for ( size_t i = 1u ; i < count ; ++i ) {
                size_t n = count - i - 1u;
                current = this->head;
                for ( size_t j = 0u; j <= n; ++j ) {
                    xHandle.n1 = current;
                    xHandle.n2 = current->next;
                    xRetCmp = f( &xHandle );
                    if ( true == xRetCmp ) { /*compare adjacent nodes*/
                        before = current->prev;
                        after = current->next;

                        if ( nullptr != before ) {
                            before->next = after;
                        }
                        else {
                            /*
                            In case <before> pointer is null, <after> pointer
                            should be the new head
                            */
                            this->head = after;
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
                        /*
                        Go to next node only if :
                        current->data > current->next->data
                        condition is false.
                        */
                        current = current->next;
                    }
                }
            }
            /*loop remaining nodes until find the new tail*/
            while ( nullptr != current->next ) {
                current = current->next;
            }
            this->tail = current;
        }
    }

    return retValue;
}
/*============================================================================*/
bool list::swap( void* node1, void* node2 )
{
    bool retValue = false;

    if ( ( nullptr != node1 ) && ( nullptr != node2 ) && ( node1 != node2 ) ) {
        node *n1 = static_cast<node*>( node1 );
        node *n2 = static_cast<node*>( node2 );

        if ( ( this == n1->container ) && ( n1->container == n2->container ) ) {
            /*nodes are part of the same list*/
            node *tmp1, *tmp2;

            if ( n2->next == n1 ) {
                n1 = static_cast<node*>( node2 );
                n2 = static_cast<node*>( node1 );
            }
            tmp1 = n1->prev; /*save links prior swap*/
            tmp2 = n2->next;
            givenNodeSwapBoundaries( n1, n2 );
            givenNodeSwapAdjacent( n1, n2 );
            n2->prev = tmp1; /*restore previously saved links*/
            n1->next = tmp2;
            givenNodesUpdateOuterLinks( n1, n2 );
            retValue = true;
        }
    }

    return retValue;
}
/*============================================================================*/
void list::givenNodeSwapBoundaries( node *n1, node *n2 )
{
    /*update the list links*/
    if ( this->head == n1 ) {
        this->head = n2;
    }
    else if ( this->head == n2 ) {
        this->head = n1;
    }
    else {
        /*nothing to do here*/
    }
    if ( this->tail == n1 ) {
        this->tail = n2;
    }
    else if ( this->tail == n2 ) {
        this->tail = n1;
    }
    else {
        /*nothing to do here*/
    }
}
/*============================================================================*/
void list::givenNodeSwapAdjacent( node *n1, node *n2 )
{
    /*check for adjacent nodes*/
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
void list::givenNodesUpdateOuterLinks( node *n1, node *n2 )
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
bool list::move( list *src, const listPosition p)
{
    bool retValue = false;

    if ( ( nullptr != src ) && ( p >= listPosition::AT_BACK ) ) {
        if ( nullptr != src->head) { /*source has items*/
            node *iNode;
            for ( listIterator i = src->begin() ; i.until() ; i++ ) {
                iNode = i.get<node*>();
                iNode->container = this;
            }
            if ( nullptr == this->head ) { /*destination is empty*/
                this->head = src->head;
                this->tail = src->tail;
            }
            else if ( AT_FRONT == p ) {
                src->tail->next = this->head;
                this->head->prev = src->tail;
                this->head = src->head;
            }
            else if ( AT_BACK == p ) {
                this->tail->next = src->head;
                src->head->prev = this->tail;
                this->tail = src->tail;
            }
            else { /*insert the new list after the position*/
                iNode = this->getNodeAtIndex( p );
                src->tail->next = iNode->next;
                src->head->prev = iNode;
                iNode->next = src->head;
            }
            this->size += src->size;
            src->clean(); /*clean up source*/
            retValue = true;
        }
    }

    return retValue;
}
/*============================================================================*/
void list::clean( void )
{
    this->head = nullptr;
    this->tail = nullptr;
    this->size = 0u;
}
/*============================================================================*/
listIterator list::begin( void ) 
{
    listIterator it( this, FORWARD, nullptr );
    return it;
}
/*============================================================================*/
listIterator list::end( void ) 
{
    listIterator it( this, BACKWARD, nullptr );
    return it;
}
/*============================================================================*/
listIterator list::from( void *offset )
{
    listIterator it( this, FORWARD, offset );
    return it;
}
/*============================================================================*/
listIterator::listIterator( list *xList, listDirection dir, void *nodeOffset )
{
    node *ret, *offset = static_cast<node*>( nodeOffset );
    l = xList;
    if ( FORWARD == dir ) {
        ret = ( xList->isMember( nodeOffset) ) ? offset : xList->head;
        iter = ( nullptr != ret ) ? ret->next : nullptr;
    }
    else {
        ret = ( xList->isMember( nodeOffset ) ) ? offset : xList->tail;
        iter = ( nullptr != ret ) ? ret->prev : nullptr;
    }
    current = static_cast<void*>( ret );
}
/*============================================================================*/
bool listIterator::until( void )
{
    return ( nullptr != current );
}
/*============================================================================*/
bool listIterator::until( void* node )
{
    return ( l->isMember( node) ) && ( node != current );
}
/*============================================================================*/
listIterator& listIterator::operator++( int )
{
    node *ret = iter;
    iter = ( nullptr != iter ) ? iter->next : nullptr;
    current = static_cast<void*>( ret );
    return *this;
}
/*============================================================================*/
listIterator& listIterator::operator--( int )
{
    node *ret = iter;
    iter = ( nullptr != iter ) ? iter->prev : nullptr;
    current = static_cast<void*>( ret );
    return *this;
}
/*============================================================================*/