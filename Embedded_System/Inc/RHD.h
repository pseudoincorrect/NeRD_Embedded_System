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


// channel number with a left shift (channel << 8) to speed-up SPI protocol
// ex : channel = 3  thus CHANNEL = channel << 8 
//     (channel)  0x0003   ==>  (CHANNEL) 0x0300
// CAREFUL : don't forget to set the amplifier POWER-UP regsiter in rhd2000.h
#define CHANNEL0			0x0000
#define CHANNEL1			0x0100
#define CHANNEL2			0x0200	
#define CHANNEL3			0x0300
#define CHANNEL4			0x0400
#define CHANNEL5			0x0500
#define CHANNEL6			0x0600
#define CHANNEL7			0x0700


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
