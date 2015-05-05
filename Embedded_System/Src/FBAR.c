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
Delta	 :			|---------|

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
		etaSous[i] = ETA / (i + 1);
		etaAdd[i]  = ETA / (CUT_VAL_SIZE - i);
	}
}

/**************************************************************/
//					FBAR_Reset
/**************************************************************/
void FBAR_Reset(uint16_t * bufferFrom, uint8_t * bufferTo)
{
	static uint16_t i,j, valueFrom;
	
	*bufferTo++ = 0xFF;
	*bufferTo++ = 0xFF;
	
	#pragma unroll_completely
	for(i=0; i < CHANNEL_SIZE; i++)
	{		
		valueFrom = *bufferFrom++;
		*bufferTo++ = (valueFrom >> 8);
		*bufferTo++ = (valueFrom & 0xFF);		
		
		delta = (cutValue[i][6]-cutValue[i][0]) / (CUT_VAL_SIZE - 1);
		
		#pragma unroll_completely 
		for(j=0; j < CUT_VAL_SIZE; j++)
			cutValue[i][j] = valueFrom + (j-3) * delta; 
		
	}
}

/**************************************************************/
//					FBAR_Compress
/**************************************************************/
void FBAR_Compress(uint16_t * bufferFrom, uint8_t * bufferTo)
{
	uint16_t i,j,winner,ValueCurrentChannel;

	#pragma unroll_completely
	for(i=0; i < CHANNEL_SIZE; i++)
	{		
		ValueCurrentChannel = *bufferFrom++;
		winner = 0;
		
		#pragma unroll_completely 
		for (j=0; j < CUT_VAL_SIZE; j++)
		{
			if (ValueCurrentChannel >= cutValue[i][j])
			{
				if( (j == (CUT_VAL_SIZE-1)  || (cutValue[i][j] - cutValue[i][j-1] >= etaAdd[j])))
					cutValue[i][j] += etaAdd[j];	
				winner = j;
			}
			else	
			{
				if( (!j || (cutValue[i][j] - cutValue[i][j-1] >= etaSous[j])))
					cutValue[i][j] -= etaSous[j];	
			}
		}
		*bufferTo++ = 0x00FF & winner; 
	}
}

/**************************************************************/
//					FBAR_Dissemble
/**************************************************************/
void FBAR_Dissemble(uint16_t * bufferFrom, uint8_t * bufferTo)
{
	uint16_t i;
	
	#pragma unroll_completely 
	for(i=0; i < (CHANNEL_SIZE/2); i++)
	{
		*bufferTo = *bufferFrom >> 8;
		
		bufferTo++;
		
		*bufferTo = (*bufferFrom) & 0xFF;
		
		bufferTo++;
		bufferFrom++;
	}
}



