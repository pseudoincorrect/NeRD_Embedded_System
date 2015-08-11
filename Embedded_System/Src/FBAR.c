#include "FBAR.h"

#if (NBIT == 4)
volatile uint16_t cutValue[CHANNEL_SIZE][1] = {0}; 
#else
volatile uint16_t cutValue[CHANNEL_SIZE][CUT_VAL_SIZE + 1] = {0}; // (CUT_VAL_SIZE + 1) : we add 1 to avoid the warning out of range line 115
#endif

#if (NBIT == 4)
volatile uint16_t Fbar_EtaAdd[4]={0};
#else
volatile uint16_t Fbar_EtaAdd[CUT_VAL_SIZE]={0};
volatile uint16_t Fbar_EtaSous[CUT_VAL_SIZE]={0};
#endif	
uint16_t range; // range RHD ADC ==> 2^16 - 1 
uint16_t delta; // initial resolution compression = range / number of cut value
volatile uint16_t Fbar_Eta;
volatile uint16_t nbit, pow2, cutvalsize;

/*	Example : N = 2 ==> 2 bits ==> 3 cuts values ==> 4 winners
						  CUT_VAL_SIZE = 3   ==> (2^2 - 1)

cut val i :							0					1					2 	
							|---------|---------|---------|---------|
winner :					00				01				10				11
Delta	 :			|---------|

Fbar_EtaAdd  [0] = Fbar_Eta/1			[1] =	Fbar_Eta/2			[2] =	Fbar_Eta/3		
Fbar_EtaSous [0] = Fbar_Eta/3			[1] =	Fbar_Eta/2			[2] =	Fbar_Eta/1	
*/

/**************************************************************************/
//					FBAR_Init
/**************************************************************************/
void FBAR_Init(uint8_t Fbar_EtaIndex)
{
	uint16_t i,j;
  
	if (Fbar_EtaIndex < 100)
    Fbar_EtaIndex = 100;
  
  Fbar_Eta = (Fbar_EtaIndex - 100) * 50;
  
	nbit = NBIT;	
	pow2 = POW_2_NBIT;
	cutvalsize = CUT_VAL_SIZE;
	
	range = 65535;
	delta = range / (CUT_VAL_SIZE + 1);
	
	// initialize the first cutvalues
	for(i=0; i < CHANNEL_SIZE; i++)
		for(j=0; j < CUT_VAL_SIZE; j++)
		{
      #if ((NBIT == 1) || (NBIT == 4))
      cutValue[i][0] = 30000;
      #else
      cutValue[i][j] = (j+1) * delta;
      #endif
    }
    
  #if ((NBIT == 2) || (NBIT == 3)) 	
	//initialize the adaptation parameters
	for (i=0; i < CUT_VAL_SIZE; i++)
	{
		Fbar_EtaSous[i] = Fbar_Eta / (i + 1);
		Fbar_EtaAdd[i]  = Fbar_Eta / (CUT_VAL_SIZE - i);
	}
  #elif (NBIT == 4)
  for (i=0; i <4; i++)
		Fbar_EtaAdd[i] = (Fbar_Eta / 3) * i;   
  #endif
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
  
    #if ((NBIT == 1) || (NBIT == 4))
    cutValue[i][0] = valueFrom;
    
		#else
    #pragma unroll_completely
    for(j=0; j < CUT_VAL_SIZE; j++)
		  cutValue[i][j] = valueFrom + (j-NBIT) * delta;
    // quand je prend un delta non constant, le système à la réception ne fonctionne pas très bien
		delta = 2000 / (CUT_VAL_SIZE - 1);  //(cutValue[i][CUT_VAL_SIZE - 1]-cutValue[i][0]) / (CUT_VAL_SIZE - 1);
    #endif 
	}
}

/**************************************************************************/
//					FBAR_Compress
/**************************************************************************/
// this function will take a pointer pointing toward 
// CHANNEL_SIZE values compress them on NBIT bits 
// and send it to a buffer (the NRF buffer) for a next sending
/**************************************************************************/
//************************************************************************* N == 1 
#if (NBIT == 1)
void FBAR_Compress(uint16_t * bufferFrom, uint8_t * bufferTo)
{
	volatile uint16_t i,j,winner,ValueCurrentChannel, tempValue;

  #pragma unroll_completely
	for(i=0; i < CHANNEL_SIZE; i++)
	{		
		ValueCurrentChannel = *bufferFrom++;  
    winner = 0;
    //Winner choosing regarding the current value and the only cut value
    if (ValueCurrentChannel >= cutValue[i][0])
    {
      if (cutValue[i][0] < 65535 - Fbar_Eta)
        cutValue[i][0] += Fbar_Eta;  
      winner = 1;
    }
    else
    {
      if (cutValue[i][0] > Fbar_Eta)
        cutValue[i][0] -= Fbar_Eta;  
    }
    //send winner to the circular buffer
    *bufferTo++ = winner; 	
	}
}
#endif

//************************************************************************* N == 2
#if ((NBIT == 2) | (NBIT == 3)) 
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
					// on controle que les cutvalues ne dépassent pas l2^16
          if (cutValue[i][CUT_VAL_SIZE-1] < 65535 - Fbar_Eta) 
						cutValue[i][CUT_VAL_SIZE-1] += Fbar_Eta;
				}
				// anti chevauchement (et compilation warning j+1, depassement range tableau, secu à prévoir)
				else 
          if((cutValue[i][j+1] - cutValue[i][j]) >= Fbar_EtaAdd[j]) 
				{
					cutValue[i][j] += Fbar_EtaAdd[j];	
				}
				winner = j+1; 
			}
			else	
			{
				if (!j)
				{
					// on controle que les cutvalues ne dépassent pas 0 en négatif
           if (cutValue[i][0] > Fbar_Eta)  
						cutValue[i][0] -= Fbar_Eta;        
				}
				// anti chevauchement (et compilation warning j-1, depassement negatif range tableau, secu à prévoir)
				else if (((cutValue[i][j] - cutValue[i][j-1]) >= Fbar_EtaSous[j]))  
				{
					cutValue[i][j] -=  Fbar_EtaSous[j];	
				}
			}
		}	
		*bufferTo++ = winner; 
	}
}
#endif

//************************************************************************* N == 4
#if (NBIT == 4)
void FBAR_Compress(uint16_t * bufferFrom, uint8_t * bufferTo)
{
	volatile uint16_t i,j,winner,ValueCurrentChannel, tempValue;

  #pragma unroll_completely
	for(i=0; i < CHANNEL_SIZE; i++)
	{		
		ValueCurrentChannel = *bufferFrom++;    
  
    if (ValueCurrentChannel >= cutValue[i][0])
    {
      tempValue = ValueCurrentChannel - cutValue[i][0];
    
      if (tempValue < Fbar_EtaAdd[1])
        winner = 0x00;
      else if (tempValue < Fbar_EtaAdd[2])
        winner = 0x01;
      else if (tempValue < Fbar_EtaAdd[3])
        winner = 0x02;
      else 
        winner = 0x03;   

      if (cutValue[i][0] < 65535 - Fbar_Eta)
        cutValue[i][0] += Fbar_EtaAdd[(winner & 0x03)];
      else 
        winner = 0x00;      
    } 
    else
    {
      tempValue = cutValue[i][0] - ValueCurrentChannel;
            
      if (tempValue < Fbar_EtaAdd[1])
        winner = 0x00 | 0x04;
      else if (tempValue < Fbar_EtaAdd[2])
        winner = 0x01 | 0x04;
      else if (tempValue < Fbar_EtaAdd[3])
        winner = 0x02 | 0x04;
      else 
        winner = 0x03 | 0x04;   
      
      if (cutValue[i][0] > Fbar_Eta) 
        cutValue[i][0] -= Fbar_EtaAdd[(winner & 0x03)];
      else 
        winner = 0x04;  
    }
      
    //send winner to the circular buffer
    *bufferTo++ = winner; 	
	}
}
#endif

/**************************************************************************/
//					FBAR_Dissemble
/**************************************************************************/
// function used in NO-compression mode, it will merge the  
// uint8_t data into a 16bit array
/**************************************************************************/
void FBAR_Dissemble(uint16_t * bufferFrom, uint8_t * bufferTo, DataStateTypeDef DataState)
{
	uint16_t i;
	
  switch (DataState)
  {
    
    case(__8ch_8bit__20kHz_NC__) : 
      #pragma unroll_completely 
      for(i=0; i < (CHANNEL_SIZE); i++)
      {
        *bufferTo = (*bufferFrom >> 8)  & 0xFF;
        
        bufferTo++;
        bufferFrom++;
      }
      break;
    
    case(__4ch_16bit_20kHz_NC__) :
     #pragma unroll_completely 
      for(i=0; i < (CHANNEL_SIZE/2); i++)
      {
        *bufferTo = (*bufferFrom >> 8)  & 0xFF;
        
        bufferTo++;
        
        *bufferTo = (*bufferFrom) & 0xFF;
        
        bufferTo++;
        bufferFrom++;
      } 
      bufferFrom+= CHANNEL_SIZE/2;
      break;
      
    case(__8ch_16bit_10kHz_NC__) :
      #pragma unroll_completely 
      for(i=0; i < (CHANNEL_SIZE/2); i++)
      {
        *bufferTo = (*bufferFrom >> 8)  & 0xFF;
        
        bufferTo++;
        
        *bufferTo = (*bufferFrom) & 0xFF;
        
        bufferTo++;
        bufferFrom++;
      } 
      break;      
    
    default :
      break;      
 
  }
}



