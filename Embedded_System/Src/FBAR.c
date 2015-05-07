#include "FBAR.h"

volatile uint16_t cutValue[CHANNEL_SIZE][CUT_VAL_SIZE] = {0};

volatile uint16_t etaAdd[CUT_VAL_SIZE]={0};
volatile uint16_t etaSous[CUT_VAL_SIZE]={0};
	
uint16_t range; // range RHD ADC ==> 2^16 - 1 
uint16_t delta; // initial resolution compression = range / number of cut value

volatile uint16_t nbit, pow2, cutvalsize;

/*	Example : N = 2 ==> 2 bits ==> 3 cuts values ==> 4 winners
						  CUT_VAL_SIZE = 3   ==> (2^2 - 1)

cut val i :							0					1					2 	
							|---------|---------|---------|---------|
winner :					00				01				10				11
Delta	 :			|---------|

etaAdd  [0] = ETA/1			[1] =	ETA/2			[2] =	ETA/3		
etaSous [0] = ETA/3			[1] =	ETA/2			[2] =	ETA/1	
*/

/**************************************************************************/
//					FBAR_Init
/**************************************************************************/
void FBAR_Init(void)
{
	uint16_t i,j;
	
	nbit = NBIT;	
	pow2 = POW_2_NBIT;
	cutvalsize = CUT_VAL_SIZE;
	
	range = 65535;
	delta = range / (CUT_VAL_SIZE + 1);
	
	// initialize the first cutvalues
	for(i=0; i < CHANNEL_SIZE; i++)
		for(j=0; j < CUT_VAL_SIZE; j++)
			cutValue[i][j] = (j+1) * delta;
	
	//initialize the adaptation parameters
	for (i=0; i < CUT_VAL_SIZE; i++)
	{
		etaSous[i] = ETA / (i + 1);
		etaAdd[i]  = ETA / (CUT_VAL_SIZE - i);
	}
}

/**************************************************************************/
//					FBAR_Reset
/**************************************************************************/
// this function will reinialise the cutvalues from the current value 
// for each channel and will send the current channel's values 
// to the base system by creating a frame comporting  a balise 
//(first two bytes = 0xFF and then the values of every channel on 16 bits) 
//before being sent to the NRF
/**************************************************************************/
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
		
		// quand je prend un delta non constant, le système à la réception ne fonctionne pas très bien
		delta = 2000 / (CUT_VAL_SIZE - 1); // (cutValue[i][CUT_VAL_SIZE - 1]-cutValue[i][0]) / (CUT_VAL_SIZE - 1);
		
		#pragma unroll_completely 
		for(j=0; j < CUT_VAL_SIZE; j++)
			cutValue[i][j] = valueFrom + (j-3) * delta; 	
	}
}

/**************************************************************************/
//					FBAR_Compress
/**************************************************************************/
// this function will take a pointer pointing toward 
//CHANNEL_SIZE values compress them on NBIT bits 
// and send it to a buffer (the NRF buffer) for a next sending
/**************************************************************************/
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
				if(j == (CUT_VAL_SIZE-1))
				{
					//on controle que les cutvalues ne dépassent pas l2^16
					if (cutValue[i][CUT_VAL_SIZE-1] <  65500 - ETA) 
						cutValue[i][j] += etaAdd[j];
				}
				//anti chevauchement (et compilation warning j+1, depassement range tableau, secu à prévoir)
				else if(cutValue[i][j+1] - cutValue[i][j] >= etaAdd[j]) 
				{
					cutValue[i][j] += etaAdd[j];	
				}
				winner = j+1; 
			}
			else	
			{
				if (!j)
				{
					//on controle que les cutvalues ne dépassent pas 0 en négatif
					if (cutValue[i][0] >  ETA) 
						cutValue[i][j] -= etaSous[j];	
				}
				//anti chevauchement (et compilation warning j-1, depassement negatif range tableau, secu à prévoir)
				else if ((cutValue[i][j] - cutValue[i][j-1] >= etaSous[j]))  
				{
					cutValue[i][j] -= etaSous[j];	
				}
			}
		}	
		*bufferTo++ = winner; 
	}
}

/**************************************************************************/
//					FBAR_Dissemble
/**************************************************************************/
// function used in NO-compression mode, it will merge the  
// uint8_t data into a 16bit array
/**************************************************************************/
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



