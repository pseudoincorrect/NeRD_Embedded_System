#include "DataBuffer.h"

DataBuffer ElectrophyData;

/**************************************************************/
//					DataBuffer_Init
/**************************************************************/
void DataBuffer_Init(void)
{
	ElectrophyData.writeRHD_index 	= 0;
	ElectrophyData.writeRHD_element = 0;
	ElectrophyData.readRHD_index 		= 0;
	ElectrophyData.writeNRF_index 	= 0;
	ElectrophyData.readNRF_index 		= 0;
}

/**************************************************************/
//					DataBuffer_WriteRHD
/**************************************************************/
uint16_t * DataBuffer_WriteRHD(void)
{	
	ElectrophyData.writeRHD_element += CHANNEL_SIZE;
	
	if(ElectrophyData.writeRHD_element >= SAMPLE_BUFFER_SIZE)
	{
		ElectrophyData.writeRHD_element  = 0;
		ElectrophyData.writeRHD_index++;
		if(ElectrophyData.writeRHD_index >= SIZE_BUFFER_RHD)
			ElectrophyData.writeRHD_index = 0;
	}
	return &(ElectrophyData.Data16[ElectrophyData.writeRHD_index][ElectrophyData.writeRHD_element]); 
}

uint16_t previousReadRHD_index;
/**************************************************************/
//					DataBuffer_ReadRHD
/**************************************************************/
uint16_t * DataBuffer_ReadRHD(void)
{
	previousReadRHD_index = ElectrophyData.readRHD_index;
	ElectrophyData.readRHD_index++;
	
	if(ElectrophyData.writeRHD_element >= SIZE_BUFFER_RHD)
		ElectrophyData.writeRHD_element = 0;
	
	return (ElectrophyData.Data16[previousReadRHD_index]); 
}

/**************************************************************/
//					DataBuffer_Data16_CheckFill
/**************************************************************/	
uint8_t DataBuffer_Data16_CheckFill(void)
{
	if (ElectrophyData.writeRHD_index != ElectrophyData.readRHD_index)
		return 1;
	else
		return 0;
}

/**************************************************************/
//					DataBuffer_WriteNRF
/**************************************************************/
uint8_t * DataBuffer_WriteNRF(void)
{	
	ElectrophyData.writeNRF_index++;
	
	if(ElectrophyData.writeNRF_index >= SIZE_BUFFER_NRF)
		ElectrophyData.writeNRF_index = 0;
		
	return (ElectrophyData.Data8[ElectrophyData.writeNRF_index]); 
}

uint16_t previousReadNRF_index;
/**************************************************************/
//					DataBuffer_ReadNRF
/**************************************************************/
uint8_t * DataBuffer_ReadNRF(void)
{	
	previousReadNRF_index = ElectrophyData.readNRF_index;
	ElectrophyData.readNRF_index++;
	
	if(ElectrophyData.readNRF_index >= SIZE_BUFFER_NRF)
		ElectrophyData.readNRF_index  = 0;
		
	return (ElectrophyData.Data8[previousReadNRF_index]); 
}

/**************************************************************/
//					DataBuffer_Data8_CheckFill
/**************************************************************/
uint8_t DataBuffer_Data8_CheckFill(void)
{
	if (ElectrophyData.readNRF_index != ElectrophyData.writeNRF_index)
		return 1;
	else
		return 0;
}
































