#include "activeObject.h"
#include <stdio.h>
#define AO_QUEUE_SIZE 6

void ActiveObject_Ctor(ActiveObject * const me, ThreadHandler threadFunction)
{
	me->aoQueue = queue_create(AO_QUEUE_SIZE);
	if(me->aoQueue == NULL)
	{
		printf("Queue buffer full\n");
		return;
	}
	me->aoThread = threadFunction;
}

//function to start active object
void ActiveObject_Start(ActiveObject * const me)
{
	me->aoThread(me);//call threadFunction (user defined)
}

//function to enqueue tasks to active object
void ActiveObject_Enqueue(ActiveObject * const me, void * request)
{
	queue_enqueue(me->aoQueue, request);
}

