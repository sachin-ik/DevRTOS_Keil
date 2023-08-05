#include "list.h"
#include "devRtosConfig.h"


#define MAX_LIST (MAX_TASKS+1)
static list_t listNodes[MAX_LIST];
static ui_Type listFreeBits = 0x0;

list_t *list_createNode(ui_Type value)
{
	 int i = 0;
	 list_t* node;
	 for(i=0;i<MAX_LIST; i++)
	 {
		 if((listFreeBits & 0x1<<i) == 0)
		 {
			 node = &listNodes[i];
			 listFreeBits |= (0x1<<i);
			 break;
		 }
	 }
	 node->index = i;
   node->data = value;
	 node->next = NULL;
	 node->prev = NULL;
	 return node;
}

// p<-c->n
// Null<-h<->p1(100)<->p2(50)<->p3(50)->Null
void list_insertNode(list_t *head, list_t *node)
{
	list_t *next = head->next;
	if(next == NULL)
	{
		head->next = node;
		node->prev = head;
		return;
	}
	while((next->next != NULL) && node->data > next->data)
	{
		node->data -= next->data;
		next = next->next;
	}
	if(next->next == NULL)
	{
		if(node->data > next->data)
		{
			node->data -= next->data; 
			next->next = node;  //h<->p1->node->Null
			node->prev = next;  //h<->p1<->node->NULL
		}
		else
		{
			next->data -= node->data; //h<->p1<->p2->NULL
			node->prev = next->prev;  //h<->p1<-node p2->NULL
			node->next = next;        //h<->p1<-node->p2->NULL 
			next->prev = node;        //h<->p1<-node<->p2->NULL
			node->prev->next = node;  //h<->p1<->node<->p2->NULL
		}	
	}
}


void list_deleteNode(list_t *node)
{
	listFreeBits &= ~(0x1<<node->index);
	if(node->prev != NULL)node->prev->next = node->next;
	if(node->next != NULL)node->next->prev = node->prev;
}
