/********************************************************************************
* @file    board_interface.h
* @author  Maxime Clement
* @version V1.0
* @date    01-Mar-2015
* @brief   Header file of the custom board user modules.
*******************************************************************************/	

#ifndef __BOARD_INTERFACE_H__
#define __BOARD_INTERFACE_H__

#include "stm32f0xx.h"
#include "stm32f0xx_hal_gpio.h"
#include "stm32f0xx_hal_tim.h"
#include "SampleSend.h"

// Configure the clock for the periphérald
void SystemClock_Config(void);

// Handler of the systick's interrupt
void SysTick_Handler(void);

// MCU Support package initialization
void HAL_MspInit(void);

// Initialise the Leds and push button USR
void Board_Init(void);

// Initialize the GPIO
static void GpioInit(void);

static void TIMInit(uint32_t reloadValue, uint16_t prescalerValue);

void TIM3_IRQHandler(void);

// Control the green/yellow led
void Board_LedPulse(void);


#endif
