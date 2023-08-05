#ifndef DEVRTOS_H
#define DEVRTOS_H

#include<stdint.h>

#define NULL ((void*)0)
	
typedef uint32_t ui_Type;
typedef int32_t  i_Type;
typedef uint8_t  ui8_Type;
typedef int8_t   i8_Type;

typedef uint32_t tSemaphore;

#define enterCritSec() __disable_irq()
#define exitCritSec()  __enable_irq()

typedef enum{
	READY=0,
	RUNNING,
	BLOCKED
}DevRtosState;

//task control block
typedef struct
{
    void *stackPtr;     // pointer to private task state
	  DevRtosState state; // task state
		ui8_Type priority;  // priority of task; 0->highest priority
	  ui_Type tcbNumber;
} DevRtosTask_t;


//RTOS init
void DevRtosInit(void);

/*RTOS task create function
 * @param me: pointer to task control block
 * @param task: pointer to task function
 * @param stackPointer: pointer to task stack
 * @param stackSize: size of task stack
 */
void DevRtosCreateTask(DevRtosTask_t* me, void (*task)(void), void *stackPointer, uint32_t stackSizeBytes, ui8_Type priority);

//RTOS scheduler
void DevRtosScheduler(void);

//RTOS start function
void DevRtosStart(void);

//Ready Queue functions
void DevRtosReadyQueueEnqueue(DevRtosTask_t* task, ui8_Type priority);

DevRtosTask_t* DevRtosReadyQueueDequeue(ui8_Type priority);

void devRtosDelay(ui_Type numTicks);
#endif
