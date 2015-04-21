#ifndef __DATABUFFER_H__
#define __DATABUFFER_H__

#include <stdint.h>
#include "CommonDefine.h"
#include "FBAR.h"

// Handler of the buffer and its pointers
typedef struct
{
	// Buffer that contain brut RHD datas
	uint16_t Data16[SIZE_BUFFER_RHD][CHANNEL_SIZE]; 
	// Buffer that contain compressed RHD datas
	uint8_t  Data8[SIZE_BUFFER_NRF][SAMPLE_BUFFER_SIZE];

	// Indice to the current write index of the RHD buffer
	uint16_t Write16_index;
	// Indice to the current buffer the NRF wiil compress/read
	uint16_t Read16_index;
	
	// Indice to the current buffer the NRF compression program is writting in
	uint16_t Write8_index;
	// Indice to the current element's buffer the NRF compression program is writting in
	uint16_t Write8_element;
	// Indice to the current buffer the NRF is sending
	uint16_t Read8_index;
	
}DataBuffer;

//initialize pointers
void DataBuffer_Init(void);

//get pointer to write into the RHD buffer (for the SPI)
uint16_t * DataBuffer_Write16(void);

//get pointer to write into the RHD buffer (entry buffer pointer for compression)
static uint16_t * DataBufferRead16(void);

// check if one buffer is ready for the compression
uint8_t DataBuffer_Data16_CheckFill(void);

//get pointer to write into the NRF buffer (exit buffer pointer for compression))
static uint8_t *  DataBufferWrite8(void);

//get a pointer to send a buffer by the NRF
uint8_t * DataBufferRead8(void);

// check if one buffer is ready for theNRF sending
uint8_t DataBuffer_Data8_CheckFill(void);

// compress a block of datas 
void DataBuffer_Compress(void);

#endif

