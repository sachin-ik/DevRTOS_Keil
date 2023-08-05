#ifndef BSP_H
#define BSP_H

#include<stdint.h>

/* system clock tick [Hz] */
#define HW_TICKS_PER_SEC 1000U

void HwInit(void);

void HwDelay(uint32_t delay);

void HwBlueLedInit(void);
void HwBlueLedOn(void);
void HwBlueLedOff(void);

void HwGreenLedInit(void);
void HwGreenLedOn(void);
void HwGreenLedOff(void);

void HwRedLedInit(void);
void HwRedLedOn(void);
void HwRedLedOff(void);

void HwButton1Init(void);
uint8_t HwButton1Read(void);
void HwButton1InterruptEnable(void);

void HwButton2Init(void);
uint8_t HwButton2Read(void);

void HwTimerInit(void);
void HwTimerStart(void);
void HwTimerStop(void);

void HwUartInit(void);
void HwUartSendByte(int c);
uint8_t HwUartGetByte(void);
void Button_Pressed(void);

#endif
