#ifndef __SBUF_H__
#define __SBUF_H__

#include "csapp.h"

struct sbuf_t{
    int *buf;                                                                   //Buffer array
    int n;                                                                      //Max num slots
    int front;                                                                  //buf[(front+1)%n] if first item
    int rear;                                                                   //buf[rear%n] is last item
    sem_t mutex;                                                                //Protects accesses to bu
    sem_t slots;                                                                //Counts available slots
    sem_t items;                                                                //Counts available items
};
typedef struct sbuf_t sbuf_t;

void sbuf_init(sbuf_t *sp, int n);
void sbuf_deinit(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp, int item);
int sbuf_remove(sbuf_t *sp);


#endif
