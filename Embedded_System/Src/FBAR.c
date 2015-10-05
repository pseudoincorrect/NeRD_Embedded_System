#include "FBAR.h"



// *************************************************************************
// *************************************************************************
// 						static function declaration, see .h file	
// *************************************************************************
// *************************************************************************
static uint8_t FBAR_AdaptCutValues(uint8_t channel, int16_t Value);
  
// *************************************************************************
// *************************************************************************
// 						static variables	
// *************************************************************************
// *************************************************************************
static uint16_t EtaAdd[CUT_VAL_SIZE]    = {0};
static uint16_t EtaSous[CUT_VAL_SIZE]   = {0};
static uint16_t PrevPrediction[CHANNEL_SIZE] = {0};

static int16_t cutValue[CHANNEL_SIZE][CUT_VAL_SIZE + 1]     = {0}; // (CUT_VAL_SIZE + 1) : we add 1 to avoid the warning out of range line 115
static int16_t cutValueSave[CHANNEL_SIZE][CUT_VAL_SIZE + 1] = {0}; // (CUT_VAL_SIZE + 1) : we add 1 to avoid the warning out of range line 11
static uint16_t Delta           = 0; // initial resolution compression = range / number of cut value
static uint16_t PredictorError  = 0;
static uint16_t Eta             = 0;
static uint16_t Beta            = 1;


/*	Example : N = 2 ==> 2 bits ==> 3 cuts values ==> 4 winners
						  CUT_VAL_SIZE = 3   ==> (2^2 - 1)

cut val i :							0					1					2 	
							|---------|---------|---------|---------|
winner :					00				01				10				11
Delta	 :			|---------|

EtaAdd  [0] = Eta/1			[1] =	Eta/2			[2] =	Eta/3		
EtaSous [0] = Eta/3			[1] =	Eta/2			[2] =	Eta/1	
*/

//*************************************************************************
//*************************************************************************
// 										Function definitions																 
//*************************************************************************
//*************************************************************************
/**************************************************************************/
//					FBAR_Init
/**************************************************************************/
void FBAR_Init(uint8_t EtaIndex)
{
	int16_t i,j;
  
	if (EtaIndex < 100)
    EtaIndex = 100; 
  
  Eta = 20; //(EtaIndex - 100) * 10;
	
	Delta = 250;
	
	// initialize the first cutvalues
	for(i=0; i < CHANNEL_SIZE; i++) 
	{
    for(j=0; j < CUT_VAL_SIZE; j++)  
    {
      cutValue[i][j] = (j-NBIT) * Delta;
      cutValueSave[i][j] = (j-NBIT) * Delta;
    }
  }
  
  for (i=0; i < CUT_VAL_SIZE; i++)
	{
		EtaSous[i] = Eta / (i + 1);
		EtaAdd[i]  = Eta / (CUT_VAL_SIZE - i);
	} 
}

/**************************************************************************/
//					FBAR_Reset
/**************************************************************************
/ this function will reinialise the cutvalues from the current value 
/ for each channel and will send the current channel's values 
/ to the base system by creating a frame comporting  a balise 
/ (first two bytes = 0xFF and then the values of every channel on 16 bits) 
/ before being sent to the NRF
**************************************************************************/
void FBAR_Reset(uint16_t * bufferFrom, uint8_t * bufferTo)
{
	static volatile uint16_t i, j, valueFrom;
  
	*bufferTo++ = 0xFF;
	*bufferTo++ = 0xFF;
	
  Delta = Eta;
  
  #pragma unroll_completely
	for(i=0; i < CHANNEL_SIZE; i++)
	{		
		valueFrom = *bufferFrom++;
		*bufferTo++ = (valueFrom >> 8);
		*bufferTo++ = (valueFrom & 0xFF);
  
    PrevPrediction[i] = valueFrom;
    
    #pragma unroll_completely
    for(j=0; j < CUT_VAL_SIZE; j++)
		  cutValue[i][j] = (j-1) * Delta; 
	}
} 

//int16_t dumbArrayX[100], dumbArrayXchapeau[100], dumbArrayWinner[100], iX, iXchapeau, iWinner ; 
/**************************************************************************/
//					FBAR_Compress
/**************************************************************************
// this function will take a pointer pointing toward 
// CHANNEL_SIZE values compress them on NBIT bits 
// and send it to a buffer (the NRF buffer) for a next sending
**************************************************************************/
void FBAR_Compress(uint16_t * bufferFrom, uint8_t * bufferTo)
{
	static uint16_t i, winner, ValueCurrentChannel;
  uint32_t tempError32;
  
	#pragma unroll_completely
	for(i=0; i < CHANNEL_SIZE; i++)
	{		
		ValueCurrentChannel = *bufferFrom++;   
    
    //PredictorError = ValueCurrentChannel - PrevPrediction[i];
    tempError32 = 0x8000 + ValueCurrentChannel - PrevPrediction[i];
    
    if (tempError32 > 0xFFFF)
      tempError32 = 0xFFFF;
    
    PredictorError =  tempError32 & 0x0000FFFF;
    
    winner = FBAR_AdaptCutValues(i, (int16_t) (PredictorError - 0x8000));
 
    *bufferTo++ = winner; 
    
    if(winner == CUT_VAL_SIZE)
    {
      if (cutValue[i][CUT_VAL_SIZE-1] > 0)
        PrevPrediction[i] = ( PrevPrediction[i] + cutValue[i][CUT_VAL_SIZE-1]); 
      else
        PrevPrediction[i] = ( PrevPrediction[i] - ((uint16_t)(- cutValue[i][CUT_VAL_SIZE-1]))); 
    }
    else if (!winner)
    {
      if (cutValue[i][CUT_VAL_SIZE-1] > 0)
        PrevPrediction[i] = ( PrevPrediction[i] + cutValue[i][0]);
      else
        PrevPrediction[i] = ( PrevPrediction[i] - ((uint16_t)(- cutValue[i][0])));
    }
    else
    {
      if ((cutValue[i][winner-1] + cutValue[i][winner]) > 0)
        PrevPrediction[i] = ( PrevPrediction[i] + (cutValue[i][winner-1] + cutValue[i][winner]) /2);
      else
        PrevPrediction[i] = ( PrevPrediction[i] - (((uint16_t) (-(cutValue[i][winner-1] + cutValue[i][winner]) /2 ))));
    }  
    
    PrevPrediction[i] = PrevPrediction[i] * 120 / 128;    
	}
}

/**************************************************************************/
//					FBAR_AdaptCutValues
/**************************************************************************/
static uint8_t FBAR_AdaptCutValues(uint8_t channel, int16_t Value)
{
  static uint8_t i, winner;
  
  winner = 0;
  
  #pragma unroll_completely 
  for (i=0; i < CUT_VAL_SIZE; i++)
  {
    if (Value >= cutValue[channel][i])
    {
      if(i == (CUT_VAL_SIZE-1))
      {
        // on controle que les cutvalues ne dépassent pas l2^15 - 1 
        if (cutValue[channel][CUT_VAL_SIZE-1] < 32767 - Eta) 
          cutValue[channel][CUT_VAL_SIZE-1] += Eta;
      }
      // anti chevauchement (et compilation warning i+1, depassement range tableau, secu à prévoir)
      else 
      {  
        if ((cutValue[channel][i+1] - cutValue[channel][i]) >= EtaAdd[i]) 
          cutValue[channel][i] += EtaAdd[i];	
      }
      winner = i+1; 
    }
    else	
    {
      if (!i)
      {
        // on controle que les cutvalues ne dépassent pas 0 en négatif
        if (cutValue[channel][0] > (-32767) + Eta)  
          cutValue[channel][0] -= Eta;        
      }
      // anti chevauchement (et compilation warning i-1, depassement negatif range tableau, secu à prévoir)
      else if (((cutValue[channel][i] - cutValue[channel][i-1]) >= EtaSous[i]))  
        cutValue[channel][i] -=  EtaSous[i];	
    }      
    cutValue[channel][i] -= (cutValue[channel][i] - cutValueSave[channel][i]) / 256;
  }	
  return winner;  
}

/**************************************************************************/
//					FBAR_Dissemble
/**************************************************************************
// function used in NO-compression mode, it will merge the  
// uint8_t data into a 16bit array
**************************************************************************/
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



