/********************************************************************************
* @file    RHD.h
* @author  Maxime Clement
* @version V1.0
* @date    01-Mar-2015
* @brief   Header file of the RHD2132 module.
*******************************************************************************/

#ifndef __RHD_H__
#define __RHD_H__



#include <stdint.h>
#include "stm32f0xx.h"
#include "rhd2000.h"
#include "CommonDefine.h"
#include "FBAR.h"





/**************************************************************/
// Spi functions
/**************************************************************/
//initialize the NRF
void RHD_Init(void);
// Initialize GPIO
static void GPIOInit(void); 
// init SPI2 
static void Spi2Init(void);
// set the csn pin state
static void CsnDigitalWrite(uint8_t state);

/**************************************************************/
// Communication functions
/**************************************************************/
// send a buffer to SPI2
static void Spi2Send(uint16_t data);
// Interrput handler SPI2
void SPI2_IRQHandler(void);
// send a buffer to SPI2 and return the data
uint16_t	Spi2ReturnSend(uint16_t data);
//compress the datas
void ApplyCompressing(uint8_t * ToCompress);

/**************************************************************/
// RHD functions
/**************************************************************/
// function used to test the spi of the NRF
static void RegisterInit(void);
//sample the channels 
void RHD_Sample(uint16_t * buffer);
// offer 3 tests for the connexion : 
// 1) one same value per channel
// 2) same counter on each chanel
// 3) increasing differtent triangles per channel 
void RHD_SampleTest(uint16_t * buffer, uint8_t test); 

void RHD_Test(void);

#endif
