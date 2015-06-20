/********************************************************************************
* @file    NRF.h
* @author  Maxime Clement
* @version V1.0
* @date    01-Mar-2015
* @brief   Header file of the nrf24l01+ module.
*******************************************************************************/
	
#ifndef __NRF_H__
#define __NRF_H__

#include <stdint.h>
#include "nRF24L01.h"
#include "stm32f0xx.h"

#include "stm32f0xx_hal_gpio_ex.h"
#include "stm32f0xx_hal_dma.h"
#include "stm32f0xx_hal_spi.h"
#include "stm32f0xx_hal_rcc.h"
#include "stm32f0xx_hal_cortex.h"
#include "DataBuffer.h"
#include "CommonDefine.h"

/**************************************************************/
// Main initialization functions
/**************************************************************/
//initialize the NRF
void NRF_Init(void);

/**************************************************************/
// Hardware functions
/**************************************************************/
// Initialize GPIO 
static void GPIOInit(void);
// initialization SPI1 
static void SpiDmaInit(void);
// set the CE (chip enable) pin state
static void CeDigitalWrite(uint8_t state);
// set the csn (not slave spi enable) pin state
static void CsnDigitalWrite(uint8_t state);

/**************************************************************/
// SPI Communication functions
/**************************************************************/

static void Spi1Send(uint16_t * dataTo, uint16_t * dataFrom, uint8_t length);

static void Spi1ReturnSend(uint8_t * dataTo, uint8_t * dataFrom, uint8_t length);
// transmit and receive 8 bits datas with SPI1 
static void Spi1ReturnSend8Bit(uint8_t * dataTo, uint8_t * dataFrom, uint8_t length);
// transmit and receive 8 bits datas with SPI1 
static void Spi1Send8Bit(uint8_t * data, uint8_t length);
// transmit a 8 bits data command before a DMA transfer
static void Spi1Send8BitThenDma (uint8_t * data, uint8_t length);
// transmit data with SPI1 through DMA
static void Spi1DmaSend(uint8_t * addr);
// DMA2 Stream0 IRQ Handler  
void DMA1_Channel2_3_IRQHandler(void);
// send samples over air
void NRF_SendBuffer(uint8_t * bufferPointer);
// check the RX buffer 
void Check_Reception(void);
/**************************************************************/
// NRF functions
/**************************************************************/
// function used to test the spi of the NRF
static void RegisterInit(void);
// function used by the main program to check wether we receive order from the base
uint8_t NRF_CheckChange(void);
// getter for
DataStateTypeDef NRF_GetDataState(void);

uint8_t NRF_GetEtaIndex(void);

// test the nrf (ask for the adress pipe 2
void NRF_Test(void);

#endif









