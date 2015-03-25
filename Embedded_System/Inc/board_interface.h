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

typedef enum
{
	IDDLE					= 0x00,
	TRANSMITTING  = 0x01,
	SETTING       = 0x02,
	STOP					= 0x03,

} Board_StateTypeDef;

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

// Initialize the PWM of the led
static void PwmInit(void);

// Control the red led 
static void RedLed(uint8_t state);

// Control the green/yellow led
static void GreellowLed(uint8_t state);
	
static void Board_Leds(Board_StateTypeDef state);

#endif
