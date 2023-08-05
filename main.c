#include "bsp.h"
#include "devRtos.h"
#include "queue.h"
#include "activeObject.h"

#include<stdio.h>
/*manual context switch by changing SP address to private stack*/
//Step1: Create stack arrays for the tasks.
#define STACK_SIZE 320 //stack size of 320 bytes

static uint8_t stack1[STACK_SIZE]; 
static uint8_t stack2[STACK_SIZE];
static uint8_t stack3[STACK_SIZE];
static uint8_t stack4[STACK_SIZE];
static uint8_t idleStack[STACK_SIZE];

static DevRtosTask_t idleTask;
static DevRtosTask_t tcb1;
static DevRtosTask_t tcb2;
static DevRtosTask_t tcb3;
static DevRtosTask_t tcb4;

//semaphore to handle synchronization
tSemaphore sem1 = 0;

ActiveObject a1;

void a1Function(ActiveObject *const me)
{
	while(1)
	{
		while(me->aoQueue->size > 0)
		{
			ui_Type* event =(ui_Type*) queue_dequeue(me->aoQueue);
			switch(*event)
			{
				case 1:
					printf("BOMB ENABLED\r");
					HwRedLedOn();
					break;
				case 2:
					printf("RED LED OFF\r");
					HwRedLedOff();
					break;
				case 3:
					printf("GREEN LED ON\n");
					HwGreenLedOn();
					break;
				case 4:
					printf("GREEN LED OFF\n");
					HwGreenLedOff();
					break;
				case 5:
				//	printf("BLUE LED ON\n");
					HwBlueLedOn();
					break;
				case 6:
					//printf("BLUE LED OFF\n");
					HwBlueLedOff();
					break;
				default:
				break;
			}
		}
	}
}


int fputc(int c, FILE *stream)
{
	 (void)stream;
	 HwUartSendByte(c);
	 return c;
}

void idle_task(void)
{
	  while(1)
    {
        __wfi(); //wait for interrupt
    }
}
//Step2: Create 2 task functions T1 and T2.
ui_Type signal1,signal2, signal3;

void T1(void)
{
		ActiveObject_Start(&a1);
    while(1)
    {
    }
}

void T2(void)
{
	//ActiveObject_Start(&a2);
    while(1)
    {
				signal2 = 3;
				ActiveObject_Enqueue(&a1, &signal2);
				devRtosDelay(1000);
				signal2 = 4;
				ActiveObject_Enqueue(&a1, &signal2);
				devRtosDelay(1000);
    }
}

void T3(void)
{
	//ActiveObject_Start(&a3);
    while(1)
    {
				signal3 = 5;
				ActiveObject_Enqueue(&a1, &signal3);
				devRtosDelay(1000);
				signal3 = 6;
				ActiveObject_Enqueue(&a1, &signal3);
				devRtosDelay(1000);
    }
}

void T4(void)
{
    while(1)
    {
				signal1 = 1;
				ActiveObject_Enqueue(&a1, &signal1);
				devRtosDelay(1000);
				signal1 = 2;
				ActiveObject_Enqueue(&a1, &signal1);
				devRtosDelay(1000);
    }
}

//button pressed signal
void Button_Pressed()
{
		if(signal1 == 1)signal1 = 2;
	  else signal1=1;
	  ActiveObject_Enqueue(&a1, &signal1);
}

//Step3: Initialize the stack pointers sp1 and sp2 to the top of the stack arrays.
uint8_t *sp1 = &stack1[0];
uint8_t *sp2 = &stack2[0];
uint8_t *sp3 = &stack3[0];
uint8_t *sp4 = &stack4[0];
int main()
{
    HwInit();

    DevRtosInit();
		DevRtosCreateTask(&idleTask, &idle_task, (void *)idleStack, STACK_SIZE, 0); //stackSize in Bytes
	  DevRtosCreateTask(&tcb1, &T1, (void *)sp1, STACK_SIZE, 1); //stackSize in Bytes
    DevRtosCreateTask(&tcb2, &T2, (void *)sp2, STACK_SIZE, 2); //stackSize in Bytes
//		DevRtosCreateTask(&tcb3, &T3, (void *)sp3, STACK_SIZE, 3); //stackSize in Bytes
	//	DevRtosCreateTask(&tcb4, &T4, (void *)sp4, STACK_SIZE, 4); //stackSize in Bytes
    ActiveObject_Ctor(&a1, &a1Function);
    DevRtosScheduler();
    while(1)
    {
    //busy wait
    }
}

/*context switching using private stack
Step1: Create 2 stack arrays for 2 tasks sp1 & sp2.
Step2: Create 2 task functions T1 and T2.
Step3: Initialize the stack pointers sp1 and sp2 to the top of the stack arrays.
Step4: Push the initial values of the registers onto the stack.(Disable floating point in compiler)
Step5: At break point, update the stack pointer register to point to the top of the stack of current task function.
Step6: Push the address of the new task function onto the stack pointer register in systick handler.


Store all registers in stack as part of context switching, since context switch can happen at any time unlike function
calls where AAPCS is followed.
During switching:
Store additional registers.
Update Stack pointer sp1 after subtracting 0x20 from SP register.

During restoring:
Restore additional registers to R4 to R11 from thread stack.
Update Stack pointer Sp after adding 0x20 to thread sp.

^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
All the above process can be automated to do context switching
*/
