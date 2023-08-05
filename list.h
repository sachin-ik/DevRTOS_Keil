#ifndef LIST_H

#define LIST_H

#include "devRtos.h"
// linked list 

typedef struct list_t list_t;

struct list_t{
	ui_Type data;
	ui8_Type index;
	DevRtosTask_t* tcb;
	list_t *next;
	list_t *prev;
};

list_t *list_createNode(ui_Type value);
void list_insertNode(list_t *head, list_t *node);
void list_deleteNode(list_t *node);

#endif
