#ifndef ACTIVEOBJECT_H
#define ACTIVEOBJECT_H

#include "queue.h"
/*
ActiveObject class
*/
typedef struct ActiveObject ActiveObject;
typedef void (*ThreadHandler)(ActiveObject * const me);

enum signal{
	AO_INIT,
	AO_BUTTON_PRESS
};

typedef enum signal Event; 

struct ActiveObject
{
	queue_t* aoQueue;
	ThreadHandler aoThread;
};

void ActiveObject_Ctor(ActiveObject * const me, ThreadHandler threadFunction);
void ActiveObject_Start(ActiveObject * const me);
void ActiveObject_Enqueue(ActiveObject * const me, void * request);


#endif

