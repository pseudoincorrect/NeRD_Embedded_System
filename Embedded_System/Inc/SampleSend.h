/********************************************************************************
* @file    SampleSend.h
* @author  Maxime Clement
* @version V1.0
* @date    01-Mar-2015
* @brief   Header file of the mechanic sampling module.
*******************************************************************************/
	
#ifndef __SAMPLESEND_H__
#define __SAMPLESEND_H__

#include <stdint.h>
#include "stm32f0xx.h"
#include "stm32f0xx_hal_gpio.h"
#include "stm32f0xx_hal_tim.h"
#include "NRF.h"
#include "RHD.h"

#define CHANNEL_SIZE 	0x08 

#define SAMPLE_BUFFER_SIZE 	(BYTES_PER_FRAME * NUMBER_OF_PACKETS) // defined in NRF.h

/**************************************************************/
// Main initialization functions
/**************************************************************/
//launch initialization function for NRF, RHD, Timer2 
void SampleSend_Init(void);

/**************************************************************/
// Hardware Initialization functions
/**************************************************************/
// Initialize Timer2 
static void TIM2Init(uint32_t reloadValue, uint16_t prescalerValue);
// Timer2 handler  
void TIM2_IRQHandler(void);
// enable the acquisition
void SampleSend_Enable(uint8_t state);
/**************************************************************/
// Sample function
/**************************************************************/
// Main sampling function
void SampleSend_Acquisition(void);

#endif

