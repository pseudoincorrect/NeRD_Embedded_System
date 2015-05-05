#include "DataBuffer.h"

DataBuffer ElectrophyData;

volatile uint8_t * NRFptr;
volatile uint16_t * RHDptr;

/**************************************************************/
//					DataBuffer_Init
/**************************************************************/
void DataBuffer_Init(void)
{
	NRFptr = ElectrophyData.Data8[0];
	RHDptr = ElectrophyData.Data16[0];
	
	ElectrophyData.Write16_index  = 0;
	ElectrophyData.Read16_index 	= 0;
	
	ElectrophyData.Write8_index 	= 0;
	ElectrophyData.Write8_element	= 0;
	ElectrophyData.Read8_index 		= 0;
}

/**************************************************************/
//					DataBuffer_Data16_CheckFill
/**************************************************************/	
uint8_t DataBuffer_Data16_CheckFill(void)
{
	if (ElectrophyData.Write16_index != ElectrophyData.Read16_index)
		return 1;
	else
		return 0;
}

uint16_t previousRead16_index;
/**************************************************************/
//					DataBuffer_Read16
/**************************************************************/
static uint16_t * DataBufferRead16(void)
{
	previousRead16_index = ElectrophyData.Read16_index;
	ElectrophyData.Read16_index++;
	
	if(ElectrophyData.Read16_index >= SIZE_BUFFER_RHD)
		ElectrophyData.Read16_index = 0;
	
	return (ElectrophyData.Data16[previousRead16_index]); 
}

/**************************************************************/
//					DataBufferWrite16
/**************************************************************/
uint16_t * DataBuffer_Write16(void)
{	
	ElectrophyData.Write16_index++;
	
	if(ElectrophyData.Write16_index >= SIZE_BUFFER_RHD)
		ElectrophyData.Write16_index = 0;

	return (ElectrophyData.Data16[ElectrophyData.Write16_index]); 
}

/**************************************************************/
//					DataBuffer_Data8_CheckFill
/**************************************************************/
uint8_t DataBuffer_Data8_CheckFill(void)
{
	if (ElectrophyData.Read8_index != ElectrophyData.Write8_index)
		return 1;
	else
		return 0;
}

uint16_t previousRead8_index;
/**************************************************************/
//					DataBufferRead8
/**************************************************************/
uint8_t * DataBufferRead8(void)
{	
	previousRead8_index = ElectrophyData.Read8_index;
	ElectrophyData.Read8_index++;
	
	if(ElectrophyData.Read8_index >= SIZE_BUFFER_NRF)
		ElectrophyData.Read8_index  = 0;
		
	return (ElectrophyData.Data8[previousRead8_index]); 
}

uint16_t previousWrite8_element;
uint16_t previousWrite8_index;
/**************************************************************/
//					 DataBufferWrite8
/**************************************************************/
static uint8_t *  DataBufferWrite8(void)
{	
	previousWrite8_element = ElectrophyData.Write8_element;
	previousWrite8_index 	 = ElectrophyData.Write8_index;
	
	ElectrophyData.Write8_element += CHANNEL_SIZE;
	
	if(ElectrophyData.Write8_element > (SAMPLE_BUFFER_SIZE - CHANNEL_SIZE))
	{
		ElectrophyData.Write8_element = 0;
		ElectrophyData.Write8_index++;
	
		if(ElectrophyData.Write8_index >= SIZE_BUFFER_NRF)
			ElectrophyData.Write8_index = 0;
	}
	return (&ElectrophyData.Data8[previousWrite8_index][previousWrite8_element]); 
}

/**************************************************************/
//					 DataBuffer_ApplyReset
/**************************************************************/
void  DataBuffer_ApplyReset(void)
{	
	ElectrophyData.Write8_element += BYTES_PER_FRAME;
	
	FBAR_Reset(DataBufferRead16(), &ElectrophyData.Data8[ElectrophyData.Write8_index][0] );	; 
}

/**************************************************************/
//					DataBuffer_Process
/**************************************************************/
void DataBuffer_Process(void)
{
	if(DataBuffer_Data16_CheckFill())
	{
#ifdef COMPRESS
		DEBUG_HIGH;
		if(ElectrophyData.Write8_element)
			FBAR_Compress(DataBufferRead16(),  DataBufferWrite8() );			
		else
			DataBuffer_ApplyReset();	
		DEBUG_LOW;
#else
		DEBUG_HIGH;
		FBAR_Dissemble(DataBufferRead16(),  DataBufferWrite8() );			
		DEBUG_LOW;
#endif
		
	}
}






























