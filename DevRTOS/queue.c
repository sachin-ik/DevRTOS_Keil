#include "queue.h"

//queueBuffer to store the queue data
#define MAX_NUM_QUEUES 10
uint8_t queueBuffer[1024]; //512 bytes
uint8_t queueBufferRearPtr = 0; //number of queues created
queue_t queueHolder[MAX_NUM_QUEUES]; //max 5 queues
uint8_t queueCount = 0; //number of queues created

//create queue function
queue_t* queue_create(ui_Type capacity)
{
	  if(queueCount == MAX_NUM_QUEUES)
		{
			return NULL;
		}
    queue_t *queue = &queueHolder[queueCount++]; //queueHolder store the queue_t struct values
    queue->capacity = capacity; //initial queue capacity
    queue->size  = 0; //initial queue size
    queue->front = 0; //initial front index
    queue->rear = capacity - 1; //initial rear index
    queue->data = (void**)&queueBuffer[queueBufferRearPtr]; //address of buffer to store queue data
    queueBufferRearPtr = queueBufferRearPtr + (capacity * sizeof(void*));
    return queue;

}
void  queue_destroy(queue_t *queue)
{
    queue->capacity = 0;
    queue->size  = 0;
    queue->front = 0;
    queue->rear = 0;
    queue->data = NULL;
}

void  queue_enqueue(queue_t *queue, void *data)
{
    if (queue->size == queue->capacity)
    {
        return;
    }
    queue->rear = (queue->rear + 1) % queue->capacity; //increment rear index
    queue->data[queue->rear] = data;
    queue->size = queue->size + 1;
}

void* queue_dequeue(queue_t *queue)
{
    if (queue->size == 0)
    {
        return NULL;
    }
    void *data = queue->data[queue->front];
    queue->front = (queue->front + 1) % queue->capacity; //increment front index
    queue->size = queue->size - 1;
    return data;
}

void* queue_front(queue_t *queue)
{
    if (queue->size == 0)
    {
        return NULL;
    }
    void *data = queue->data[queue->front];
    return data;
}
