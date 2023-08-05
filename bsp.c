#include "bsp.h"
#include "TM4C123GH6PM.h"
#include "devRtos.h"
#include<stdio.h>

#define LED_RED (1U << 1) // PF1
#define LED_BLUE (1U << 2) // PF2
#define LED_GREEN (1U << 3) // PF3
#define BUTTON1 (1U << 4) // PF4
#define BUTTON2 (1U << 0) // PF0

static volatile uint32_t tickCounter = 0;

void HwInit()
{
    // Enable the system clock for GPIO ports
    SYSCTL->RCGCGPIO |= (1U << 5); // Port F

    //Enable AHB bus fpr GPIOF
    SYSCTL->GPIOHBCTL |= (1U << 5); // Port F

    // LED inits
    HwBlueLedInit();
    HwGreenLedInit();
    HwRedLedInit();
		HwUartInit();
    // system core clock update
    SystemCoreClockUpdate();

    // enable systick timer using systick_config
    SysTick_Config(SystemCoreClock / HW_TICKS_PER_SEC);
	
		NVIC_SetPriority(SysTick_IRQn, 0U);
		HwButton1Init();
		HwButton1InterruptEnable();
    __enable_irq();

}

void SysTick_Handler()
{
    ++tickCounter;
	__disable_irq();
    DevRtosScheduler();
	__enable_irq();
}

// Polling delay
void HwDelay(uint32_t delay)
{
    uint32_t i;
    for(i = 0; i < delay; i++)
    {
        // do nothing
    }
}

/* Blue LED Functions*/
void HwBlueLedInit()
{
    // Set the direction of PF2 as output
    GPIOF_AHB->DIR |= LED_BLUE;

    // Enable digital function for PF2
    GPIOF_AHB->DEN |= LED_BLUE;
}

void HwBlueLedOn()
{
    GPIOF_AHB->DATA |= LED_BLUE;
}

void HwBlueLedOff()
{
    GPIOF_AHB->DATA &= ~LED_BLUE;
}
/**********************************************/

/* Green LED Functions*/
void HwGreenLedInit()
{
    // Set the direction of PF3 as output
    GPIOF_AHB->DIR |= LED_GREEN;

    // Enable digital function for PF3
    GPIOF_AHB->DEN |= LED_GREEN;
}

void HwGreenLedOn()
{
    GPIOF_AHB->DATA |= LED_GREEN;
}

void HwGreenLedOff()
{
    GPIOF_AHB->DATA &= ~LED_GREEN;
}
/*************************************************/

/* Red LED Functions*/
void HwRedLedInit()
{
    // Set the direction of PF1 as output
    GPIOF_AHB->DIR |= LED_RED;

    // Enable digital function for PF1
    GPIOF_AHB->DEN |= LED_RED;
}

void HwRedLedOn()
{
    GPIOF_AHB->DATA |= LED_RED;
}

void HwRedLedOff()
{
    GPIOF_AHB->DATA &= ~LED_RED;
}

/***********************************************/

// HW button1 functions

// PF4
void HwButton1Init()
{
    // Set the direction of PF4 as input
    GPIOF_AHB->DIR &= ~(BUTTON1);

    // Enable digital function for PF4
    GPIOF_AHB->DEN |= (BUTTON1);

    // Enable pull up resistor for PF4
    GPIOF_AHB->PUR |= (BUTTON1);
}

uint8_t HwButton1Read()
{
    return (GPIOF_AHB->DATA & BUTTON1);
}

// HW button2 functions

// PF0
void HwButton2Init()
{
    // Set the direction of PF0 as input
    GPIOF_AHB->DIR &= ~(BUTTON2);

    // Enable digital function for PF0
    GPIOF_AHB->DEN |= (BUTTON2);

    // Enable pull up resistor for PF0
    GPIOF_AHB->PUR |= (BUTTON2);
}

uint8_t HwButton2Read()
{
    return (GPIOF_AHB->DATA & BUTTON2);
}

//HW button1 interrupt
void HwButton1InterruptEnable()
{
    // Enable interrupt for PF4
    GPIOF_AHB->IEV &= ~(BUTTON1); // Falling edge
    GPIOF_AHB->IM |= (BUTTON1); // Unmask interrupt
    NVIC_EnableIRQ(GPIOF_IRQn); // Enable interrupt in NVIC
}

void HwButton1InterruptDisable()
{
    // Disable interrupt for PF4
    GPIOF_AHB->IM &= ~(BUTTON1); // Mask interrupt
    NVIC_DisableIRQ(GPIOF_IRQn); // Disable interrupt in NVIC
}

//GPIOF interrupt handler
void GPIOF_Handler()
{
    // Clear interrupt flag
		HwDelay(1000);//delay to avoid button debounce
    GPIOF_AHB->ICR |= (BUTTON1);
		Button_Pressed();
}

// HW timer functions
void HwTimerInit()
{
    // Enable the system clock for timer0
    SYSCTL->RCGCTIMER |= (1U << 0);

    // Disable timer0
    TIMER0->CTL &= ~(1U << 0);

    // Configure timer0 mode
    TIMER0->CFG = 0x04;

    // Configure timer0 mode
    TIMER0->TAMR |= (0x02 << 0); // Periodic mode
    TIMER0->TAMR &= ~(1U << 4); // Count down

    // Set timer0 interval load value
    TIMER0->TAILR = 16000000 - 1; // 1 sec

    // Clear timeout flag
    TIMER0->ICR |= (1U << 0);

    // Enable timeout interrupt
    TIMER0->IMR |= (1U << 0);

    // Enable timer0
    TIMER0->CTL |= (1U << 0);

    // Enable timer0 interrupt in NVIC
    NVIC_EnableIRQ(TIMER0A_IRQn);
}

void HwTimerStart()
{
    // Enable timer0
    TIMER0->CTL |= (1U << 0);
}

void HwTimerStop()
{
    // Disable timer0
    TIMER0->CTL &= ~(1U << 0);
}

void TIMER0A_IRQHandler()
{
    // Clear timeout flag
    TIMER0->ICR |= (1U << 0);

    // Toggle green led
    HwGreenLedOn();
    HwDelay(100000);
    HwGreenLedOff();
}

#define UART_BAUD_RATE      115200U
#define UART_FR_TXFE        (1U << 7)
#define UART_FR_RXFE        (1U << 4)
#define UART_BUSY           (1U << 3)
#define UART_TXFF           (1U << 5)
#define UART_TXFIFO_DEPTH   16U
// HW UART functions
void HwUartInit()
{
    /* enable clock for UART0 and GPIOA (used by UART0 pins) */
    SYSCTL->RCGCUART   |= (1U << 0); /* enable Run mode for UART0 */
    SYSCTL->RCGCGPIO   |= (1U << 0); /* enable Run mode for GPIOA */

    /* configure UART0 pins for UART operation */
    uint32_t tmp = (1U << 0) | (1U << 1);
    GPIOA->DIR   &= ~tmp;
    GPIOA->SLR   &= ~tmp;
    GPIOA->ODR   &= ~tmp;
    GPIOA->PUR   &= ~tmp;
    GPIOA->PDR   &= ~tmp;
    GPIOA->AMSEL &= ~tmp;  /* disable analog function on the pins */
    GPIOA->AFSEL |= tmp;   /* enable ALT function on the pins */
    GPIOA->DEN   |= tmp;   /* enable digital I/O on the pins */
    GPIOA->PCTL  &= ~0x00U;
    GPIOA->PCTL  |= 0x11U;

    /* configure the UART for the desired baud rate, 8-N-1 operation */
    tmp = (((SystemCoreClock * 8U) / UART_BAUD_RATE) + 1U) / 2U;
    UART0->IBRD  = tmp / 64U;
    UART0->FBRD  = tmp % 64U;
    UART0->LCRH  = (0x3U << 5); /* configure 8-N-1 operation */
    UART0->CTL   = (1U << 0)    /* UART enable */
                    | (1U << 8)  /* UART TX enable */
                    | (1U << 9)  /* UART RX enable */
                    | (1U << 4); /* EOT end of transmission */
}

void HwUartSendByte(int c)
{
    /* busy-wait as long as UART busy */
    while ((UART0->FR & UART_BUSY) != 0) {
    }
    UART0->DR = c; /* write the byte into Data Register */
}

uint8_t HwUartGetByte()
{
    // Wait until there is data in the FIFO
    while((UART0->FR & (1U << 4)) != 0){}

    // Return the byte
    return UART0->DR;
}


