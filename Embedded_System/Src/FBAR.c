#include "FBAR.h"

static uint16_t cutValue[CHANNEL_SIZE][CUT_VAL_SIZE] = {0};

static uint16_t etaAdd[CUT_VAL_SIZE]={0};
static uint16_t etaSous[CUT_VAL_SIZE]={0};
	
uint16_t range; // range RHD ADC ==> 2^16 - 1 
uint16_t delta; // initial resolution compression = range / number of cut value

/*	Example : N = 2 ==> 2 bits ==> 3 cuts values ==> 4 winners
						  CUT_VAL_SIZE = 3   ==> (2^2 - 1)

cut val i :							0					1					2 	
							|---------|---------|---------|---------|
winner :					00				01				10				11
Delta					|---------|

etaAdd  [0] = ETA/1			[1] =	ETA/2			[2] =	ETA/3		
etaSous [0] = ETA/3			[1] =	ETA/2			[2] =	ETA/1	
*/

/**************************************************************/
//					FBAR_Init
/**************************************************************/
void FBAR_Init(void)
{
	uint16_t i,j;
	
	range = 65535;
	delta = range / (CUT_VAL_SIZE + 1);
	
	for(i=0; i < CHANNEL_SIZE; i++)
		for(j=0; j < CUT_VAL_SIZE; j++)
			cutValue[i][j] = (j+1) * delta;
	
	for (i=0; i < CUT_VAL_SIZE; i++)
	{
		etaSous[i] = ETA / (i+1);
		etaAdd[i]  = ETA / (CUT_VAL_SIZE-i);
	}
}

/**************************************************************/
//					FBAR_Compress
/**************************************************************/
void FBAR_Compress(uint16_t * bufferCompress, uint8_t * bufferSample, uint8_t number_channel)
{
	uint16_t i,j,winner,ValueCurrentChannel;
	
	for(i=0; i < number_channel; i++)
	{		
		ValueCurrentChannel = *bufferCompress++;
		winner = 0;
	
		for (j=0; j < CUT_VAL_SIZE; j++)
		{
			if (ValueCurrentChannel >= cutValue[i][j])
			{
				cutValue[i][j] += etaAdd[j];	
				winner = j;
			}
			else	
				cutValue[i][j] -= etaSous[j];	
		}
		*bufferSample++ = winner; 
	}
}


//routine to check and compress datas
//void	RHD_Compress(void);	

///**************************************************************/
////					RHD_Compress
///**************************************************************/
//void RHD_Compress(void)
//{
//	if(compress_Flag)
//	{
//		GPIOA->BSRR |= GPIO_PIN_15;
//		GPIOA->BSRR |= ((uint32_t) GPIO_PIN_15 << 16); 
//		
//		compress_Flag = 0;
//	
//		FBAR_Compress(pBufferCompress, bufferSample, CHANNEL_SIZE);
//		
//		GPIOA->BSRR |=  GPIO_PIN_15;
//		GPIOA->BSRR |= ((uint32_t) GPIO_PIN_15 << 16);
//	}
//}

