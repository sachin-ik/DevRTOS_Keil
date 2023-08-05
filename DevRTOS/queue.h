#ifndef QUEUE_H
#define QUEUE_H

#include "devRtos.h"

typedef struct{
    ui_Type  front;    // index of the first element in the queue
    ui_Type  rear;     // index of the next free slot
    ui_Type  size;     // current number of elements in the queue
    ui_Type  capacity; // max number of elements in the queue
    void     **data;   // array of pointers to void (generic) data
}queue_t;

queue_t* queue_create(ui_Type capacity);
void  queue_destroy(queue_t *queue);
void  queue_enqueue(queue_t *queue, void *data);
void* queue_dequeue(queue_t *queue);
void* queue_front(queue_t *queue);
#endif

