/** @file BufferQueue.h
 * 
 * @author marco corvi
 * @date jan 2009
 *
 * @brief buffer queue
 * --------------------------------------------------------
 *  Copyright This sowftare is distributed under GPL-3.0 or later
 *  See the file COPYING.
 */
#ifndef BUFFER_QUEUE_H
#define BUFFER_QUEUE_H

#include <string.h>
#include <time.h>
// #include <linux/time.h>
#ifdef PTHREAD
  #include <pthread.h>
#endif

template< typename T >
class QueueItem
{
  private:
    T m_t;
    QueueItem * next;

  public:
    QueueItem( T & t )
      : next( 0 )
    { 
      memcpy(&m_t, &t, sizeof(T));
    }

    void SetNext( QueueItem * n ) { next = n; }

    QueueItem *  Next( ) const { return next; }

    void GetItem( T & t ) const 
    { 
      memcpy(&t, &m_t, sizeof(T));
    }

};

template< typename T >
class BufferQueue 
{
  private:
    QueueItem<T> * first;
    QueueItem<T> * last;
    unsigned int   n_item;  //!< number of items on the queue
    #ifdef PTHREAD
      pthread_mutexattr_t mutex_attr;
      pthread_condattr_t  cond_attr;
      pthread_mutex_t mutex;
      pthread_cond_t  cond;
    #endif

  public:
    /** cstr
     */
    BufferQueue( )
      : first( 0 )
      , last( 0 )
      , n_item( 0 )
    {  
      #ifdef PTHREAD
        pthread_mutexattr_init( &mutex_attr );
        pthread_condattr_init( &cond_attr );
        pthread_mutex_init( &mutex, &mutex_attr );
        pthread_cond_init( &cond, &cond_attr );
      #endif
    }

    ~BufferQueue( )
    {
      QueueItem<T> * next;
      for ( QueueItem<T> * item = first; item != NULL; item = next ) {
        next = item->Next();
        delete item;
      }
      #ifdef PTHREAD
        pthread_mutex_destroy( &mutex );
        pthread_cond_destroy( &cond );
        pthread_mutexattr_destroy( &mutex_attr );
        pthread_condattr_destroy( &cond_attr );
      #endif
    }

    /** get the numbner of items on the queue
     * @return the size of the queue
     */
    unsigned int Size() const { return n_item; }

    /** put an item on the queue
     * @param t  item to put on the queue
     */
    void Put( T & t )
    {
      #ifdef PTHREAD
        pthread_mutex_lock( &mutex );
      #endif
      QueueItem<T> * item = new QueueItem<T>( t );
      if ( last != NULL ) {
        last->SetNext( item );
        last = item;
        ++ n_item;
      } else {
        if ( first != NULL ) {
          fprintf(stderr, "ERROR: BufferQueue: last is NULL but first is not NULL\n"); 
          delete item;
        } else {
          first = last = item;
          ++ n_item;
        }
        #ifdef PTHREAD
          pthread_cond_signal( &cond );
        #endif
      }
      #ifdef PTHREAD
        pthread_mutex_unlock( &mutex );
      #endif
    }

    /** get an item from the queue
     * @param t where the item from the queue is copied
     * @return true if could get an item from the queue within the timeout
     */
    bool Get ( T & t )
    {
      #ifdef PTHREAD
        pthread_mutex_lock( &mutex );
        while ( first == NULL ) {
          struct timespec wait; 
          clock_gettime(CLOCK_REALTIME, &wait);
          wait.tv_sec += 1;
          pthread_cond_timedwait( &cond, &mutex, &wait );
        }
      #endif
      if ( first == NULL ) {
        fprintf(stderr, "ERROR: BufferQueue() Get first NULL %p last %p\n", (void *)first, (void *)last);
        return false;
      }
      first->GetItem( t );
      QueueItem<T> * item = first;
      if ( last == first ) {
        first = last = NULL;
      } else {
        first = first->Next();
      }
      -- n_item;
      #ifdef PTHREAD
        pthread_mutex_unlock( &mutex );
      #endif
      delete item;
      return true;
    }

};


#endif // BUFFER_QUEUE_H

