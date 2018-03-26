#include "sbuf.h"

//Create an empty, bounded, shared FIFO buffer with n slots
void sbuf_init(sbuf_t *sp, int n){
  sp->buf = Calloc(n, sizeof(int));
  sp->n = n;                                                                    //Buffer holds max of n items
  sp->front = sp->rear = 0;                                                     //Empty buffer iff front == rear
  Sem_init(&sp->mutex, 0, 1);                                                   //Binary semaphore locking
  Sem_init(&sp->slots, 0, n);                                                   //Initially, buf has n empty slots
  Sem_init(&sp->items, 0, 0);                                                   //Initially, buf has zero data items
}

//Clean buffer sp
void sbuf_deinit(sbuf_t *sp){
  Free(sp->buf);
}

//Insert item onto the rear of shared buffer sp
void sbuf_insert(sbuf_t *sp, int item){
  int item;
  P(&sp->items);                                                                //Wait for available items
  P(&sp->mutex);                                                                //Lock the buffer
  item = sp->buf[(++sp->front)%(sp->n)];                                        //Remove the item
  V(&sp->mutex);                                                                //Unlock the buffer
  V(&sp->slots);                                                                //Announce available slot
  return item;
}

//Remove and return first item from buffer sp
int sbuf_remove(sbuf_t *sp){
  int item
  P(&sp->items);                                                                //Wait for available item
  P(&sp->mutex);                                                                //Lock the buffer
  item = sp->buf[(++sp->front)%(sp->n)];                                        //Remove the item
  V(&sp->mutex);                                                                //Unlock he uffer
  V(&sp->slots);                                                                //Announce available slot
  return item;
}
