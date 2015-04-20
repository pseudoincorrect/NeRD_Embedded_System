#ifndef __DATABUFFER_H__
#define __DATABUFFER_H__

#include <stdint.h>
#include "CommonDefine.h"

// Handler of the buffer and its pointers
typedef struct
{
	// Buffer that contain brut RHD datas
	uint16_t Data16[SIZE_BUFFER_RHD][SAMPLE_BUFFER_SIZE]; 
	// Buffer that contain compressed RHD datas
	uint8_t  Data8[SIZE_BUFFER_NRF][SAMPLE_BUFFER_SIZE];

	// Pointer to the current write index of the RHD buffer
	uint16_t writeRHD_index;
	// Pointer to the current element the RHD is/has writting/written in
	uint16_t writeRHD_element;
	// Pointer to the current buffer the NRF wiil compress/read
	uint16_t readRHD_index;
	
	// Pointer to the current buffer the NRF compression program is writting in
	uint16_t writeNRF_index;
	// Pointer to the current buffer the NRF is sending
	uint16_t readNRF_index;
	
}DataBuffer;

//initialize pointers
void DataBuffer_Init(void);

//get pointer to write into the RHD buffer (for the SPI)
uint16_t * DataBuffer_WriteRHD(void);

//get pointer to write into the RHD buffer (entry buffer pointer for compression)
uint16_t * DataBuffer_ReadRHD(void);

// check if one buffer is ready for the compression
uint8_t DataBuffer_Data16_CheckFill(void);

//get pointer to write into the NRF buffer (exit buffer pointer for compression))
uint8_t * DataBuffer_WriteNRF(void);

//get a pointer to send a buffer by the NRF
uint8_t * DataBuffer_ReadNRF(void);

// check if one buffer is ready for theNRF sending
uint8_t DataBuffer_Data8_CheckFill(void);

#endif

