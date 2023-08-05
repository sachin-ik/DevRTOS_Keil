#include "devRtos.h"
#include "devRtosConfig.h"
#include "bsp.h"
#include "queue.h"
#include "list.h"

/*-----------------------------------------------------------*/
//unsigned long ulTaskNumber[ MAX_TASKS ];

//init function to enable pendsv interrupt
DevRtosTask_t* volatile DevRtosCurTask; //pointer to current task TCB, address is volatile
DevRtosTask_t* volatile DevRtosNextTask; //pointer to next task TCB, address is volatile

// Ready queue
static queue_t* DevRtosTaskQueueArray[MAX_TASKS];
static ui_Type DevRtosTaskCount = 0;
//static ui_Type DevRtosRoundRobin = 0;
// priority: 0th bit -> priority 1, 1st bit -> priority 2 ... this is to use priority 0 as idle task
static ui_Type DevRtosReadyPriorityBits = 0;

//Delay list
static list_t* DevRtosDelayList;


//Ready Queue functions
void DevRtosReadyQueueEnqueue(DevRtosTask_t* task, ui8_Type priority)
{
	  if(priority != 0)DevRtosReadyPriorityBits |= (1<<(priority-1)); //set bit
		queue_t* DevRtosReadyQueue = DevRtosTaskQueueArray[priority];
		queue_enqueue(DevRtosReadyQueue, (void*) task);
}

DevRtosTask_t* DevRtosReadyQueueDequeue(ui8_Type priority)
{
		if(priority != 0)DevRtosReadyPriorityBits &= ~(1<<(priority-1)); //clear the priority bit
	  return (DevRtosTask_t*)queue_dequeue(DevRtosTaskQueueArray[priority]);
}

DevRtosTask_t* DevRtosReadyQueueFront(ui8_Type priority)
{
	 return (DevRtosTask_t*)queue_front(DevRtosTaskQueueArray[priority]);
}

ui_Type DevRtosReadyQueueSize(ui8_Type priority)
{
	 return DevRtosTaskQueueArray[priority]->size;
}


//Delay list functions
void devRtosDelay(ui_Type numTicks)
{
		DevRtosTask_t* tcb = DevRtosCurTask;
		list_t* node = list_createNode(numTicks);
		tcb->state = BLOCKED;
	  node->tcb = tcb;
		list_insertNode(DevRtosDelayList, node);
		__disable_irq();
		DevRtosScheduler();
	  __enable_irq();
}

void devRtosDelayDec(void)
{
		list_t* next = DevRtosDelayList->next;
		if(next)next->data--;
		while(next != NULL && next->data == 0)
		{
			next->tcb->state = READY;
			DevRtosReadyQueueEnqueue(next->tcb, next->tcb->priority);
			list_t* temp = next;
			next = next->next;
			list_deleteNode(temp);
		}
}

// DevRtos functions
void DevRtosInit(void)
{
    /*set PendSV interrupt priority to the lowest level*/
		*(uint32_t volatile *)0xE000ED20 |= (0xFFU << 16);
		
		// READY TASK queue with priorities 0 to MAX_TASKS
	  ui8_Type taskIndex = 0;
    for(taskIndex=0; taskIndex<MAX_TASKS; taskIndex++)
		{
			DevRtosTaskQueueArray[taskIndex] = queue_create(MAX_TASKS);
			//ulTaskNumber[taskIndex] = 0;
		}
		DevRtosDelayList = list_createNode(0xffffffffU);
}


//create task function
void DevRtosCreateTask(DevRtosTask_t* me, void (*task)(void), void *stackPointer, uint32_t stackSizeBytes, ui8_Type priority)
{
    uint32_t *stackLimit;
    /* round down the stack top to the 8-byte boundary 
	   * ARM stack grows from hi -> low memory
	   */
    uint32_t *stackPtr = (uint32_t *)((((uint32_t)stackPointer + stackSizeBytes)/8)*8); //point to the top of the stack, 8 byte alignment
    //push the initial values of the registers onto the stack.(Disable floating point in compiler)
    *(--stackPtr) = (1U << 24); // xPSR Thumb mode bit should be set in xpsr register
    *(--stackPtr) = (uint32_t)task; // PC pointer to task function
    *(--stackPtr) = 0x0000000EU; // LR
    *(--stackPtr) = 0x0000000CU; // R12
    *(--stackPtr) = 0x00000003U; // R3
    *(--stackPtr) = 0x00000002U; // R2
    *(--stackPtr) = 0x00000001U; // R1
    *(--stackPtr) = 0x00000000U; // R0
    //additional registers for context switching
    *(--stackPtr) = 0x0000000BU; // R11
    *(--stackPtr) = 0x0000000AU; // R10
    *(--stackPtr) = 0x00000009U; // R9
    *(--stackPtr) = 0x00000008U; // R8
    *(--stackPtr) = 0x00000007U; // R7
    *(--stackPtr) = 0x00000006U; // R6
    *(--stackPtr) = 0x00000005U; // R5
    *(--stackPtr) = 0x00000004U; // R4
    //save the top of stack and stack size in the task control block
    (me)->stackPtr = stackPtr;
    //(me)->stackSize = stackSize;

		//8 byte aligned
    stackLimit = (uint32_t*)(((((uint32_t)stackPointer - 1U)/8) + 1U)*8);

    //prefill the empty stack with 0xDEADBEEF
    while(stackPtr > stackLimit)
    {
        *(--stackPtr) = 0xDEADBEEFU;
    }

    //set the last created task as the current task
		(me)->tcbNumber = DevRtosTaskCount;
		(me)->priority = priority;
		DevRtosReadyQueueEnqueue(me, priority);
    DevRtosTaskCount++;
}


//scheduler function to switch between tasks called from systick handler
void DevRtosScheduler(void)
{
		if(DevRtosCurTask->state == BLOCKED)
		{
				//DevRtosCurTask = DevRtosReadyQueueDequeue(DevRtosCurTask->priority);
		}
		else if(DevRtosCurTask->state == RUNNING)
		{
			//enqueue again to get RoundRobin scheduling
			DevRtosReadyQueueEnqueue(DevRtosCurTask, DevRtosCurTask->priority);
			DevRtosCurTask->state = READY;
		}
		
		devRtosDelayDec();//decrement delay count
		
		ui8_Type priority = 32 - __clz(DevRtosReadyPriorityBits);//count leading zeros, gives number of zeros before set bit from MSB
		DevRtosNextTask = DevRtosReadyQueueDequeue(priority);
		if(priority==0)DevRtosReadyQueueEnqueue(DevRtosNextTask, priority); //idle task is always in ready state
	  DevRtosNextTask->state = RUNNING;  
		
    if(DevRtosNextTask != DevRtosCurTask)
    {
				//enable pendsv interrupt with address for cortex m4
				*(uint32_t volatile *)0xE000ED04 = (1U << 28);
        //pendsv handler will switch the context to the next task
    }
}

/*
void PendSV_Handler()
{
    //disable IRQ
    __asm volatile ("CPSID I");

    //if DevRtosCurTask == 0, then this is the first time the scheduler is called.
    //so we need not store the current context TCB SP
    if(DevRtosCurTask != 0)
    {
        //save the context of the current task only R4-R11. R12, r0-R3, pc, xpsr, lr pushed by hardware
        __asm volatile ("PUSH {R4-R11}"); //push the registers from R4 to R11 to scur task SP
        //load the address of DevRtosCurTask to R1
        __asm volatile ("LDR R1, =DevRtosCurTask");
        //load the value of DevRtosCurTask to R1
        __asm volatile ("LDR R1, [R1]");
        ////store the value of SP in DevRtosCurTask stack pointer
        __asm volatile ("STR SP, [R1, #0x00]"); //DevRtoscuTask->stackPtr = SP;
    }

    //load address of DevRtosNextTask to R1
    __asm volatile ("LDR R1, =DevRtosNextTask");  //R1=(DevRtosTask_t*)DevRtosNextTask 
    //load the value of DevRtosNextTask to R1
    __asm volatile ("LDR R1, [R1, #0x00]");     //R1=*DevRtosNextTask
    //load the value of DevRtosNextTask stack pointer to SP
    __asm volatile ("LDR SP, [R1, #0x00]"); //SP = (*DevRtosNextTask).stackPtr; since stackPtr is first element

    DevRtosCurTask = DevRtosNextTask; 

    //pop the registers from the new stack pointer
    __asm volatile ("POP {R4-R11}");  //pop the registers from SP to R4 to R11

    //enable IRQ
    __asm volatile ("CPSIE I");

    //return from interrupt
    __asm volatile ("BX LR");

}*/

//Keil uVision code
__asm
void PendSV_Handler()
{
    IMPORT DevRtosCurTask
    IMPORT DevRtosNextTask

    //disable IRQ
    CPSID I

    //if DevRtosCurTask == 0, then this is the first time the scheduler is called
    //so we need to initialize the stack pointer to the first task
    LDR R1, =DevRtosCurTask //load address of DevRtosCurTask to R1
    LDR R1, [R1]            //load the value of DevRtosCurTask to R1
    CBZ R1, PendSvRestore   //if R1 is zero, then jump to PendSvRestore

    //save the context of the current task only R4-R11. R12, r0-R3, pc, xpsr, lr pushed by hardware
    PUSH {R4-R11}

    //store the current task stack pointer in the task control block
    LDR R1, =DevRtosCurTask  //load address of DevRtosCurTask to R1
    LDR R1, [R1, #0x00]      //load the value of DevRtosCurTask to R1
    STR SP, [R1, #0x00]      //store the value of SP in DevRtosCurTask stack pointer
    //restore the context of the next task
PendSvRestore
    //load next task stack pointer from the task control block
    LDR R1, =DevRtosNextTask  //load address of DevRtosNextTask to R1
    LDR R1, [R1, #0x00]       //load the value of DevRtosNextTask to R1
    LDR SP, [R1, #0x00]       //load the value of DevRtosNextTask stack pointer to SP

    //store the current task in the DevRtosCurTask
    LDR R1, =DevRtosCurTask   //address of DevRtosCurTask
    LDR R2, =DevRtosNextTask  //address of DevRtosNextTask
    LDR R2, [R2]              //load the value of DevRtosNextTask
    STR R2, [R1]              //store the value of DevRtosNextTask in DevRtosCurTask

    //pop the registers from the new stack pointer
    POP {R4-R11}

    //enable IRQ
    CPSIE I

    //return from interrupt
    BX LR
}

