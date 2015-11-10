/********************************************************************************
* @file    RHD.h
* @author  Maxime Clement
* @version V1.0
* @date    01-Mar-2015
* @brief   Header file of the RHD2132 module.
*******************************************************************************/

#ifndef __RHD_H__
#define __RHD_H__

//#define SWITCH_CHANNEL

#include <stdint.h>
#include "stm32f0xx.h"
#include "rhd2000.h"
#include "CommonDefine.h"
#include "FBAR.h"


// channel number with a left shift (channel << 8) to speed-up SPI protocol
// ex : channel = 3  thus CHANNEL = channel << 8 
//     (channel)  0x0003   ==>  (CHANNEL) 0x0300
// CAREFUL : don't forget to set the amplifier POWER-UP regsiter in rhd2000.h
#ifndef SWITCH_CHANNEL
  #define CHANNEL0	0x0300	 //Omnetics_chan 10  RHD_In 3
  #define CHANNEL1	0x0400   //Omnetics_chan 14  RHD_In 4
  #define CHANNEL2	0x1F00   //Omnetics_chan 21  RHD_In 27
  #define CHANNEL3	0x1D00   //Omnetics_chan 27  RHD_In 29
  #define CHANNEL4	0x1B00   //Omnetics_chan 33  RHD_In 31
  #define CHANNEL5	0x1E00   //Omnetics_chan NC  RHD_In 30 NC
  #define CHANNEL6	0x0000   //Omnetics_chan 3   RHD_In 0 
  #define CHANNEL7	0x0200   //Omnetics_chan 8   RHD_In 2   
#else
	#define CHANNEL0	0x0500	//Omnetics_chan 5    RHD_In 5
  #define CHANNEL1	0x1A00  //Omnetics_chan 26   RHD_In 26
  #define CHANNEL2	0x1C00  //Omnetics_chan 28   RHD_In 28
  #define CHANNEL3	0x1D00  //Omnetics_chan 29   RHD_In 29
  #define CHANNEL4	0x1E00  //Omnetics_chan 30   RHD_In 30
  #define CHANNEL5	0x0000  //Omnetics_chan NC   RHD_In 0 NC
  #define CHANNEL6	0x0100  //Omnetics_chan 1    RHD_In 1
  #define CHANNEL7	0x0300  //Omnetics_chan 3    RHD_In 3
#endif

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
