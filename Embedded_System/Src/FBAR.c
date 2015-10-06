// *************************************************************************
/* ************************************************************************
  * @file    FBAR.c
  * @author  Maxime CLEMENT
  * @version V1.0
  * @date    06-Oct-2015
  * @brief   RHD2000 module driver driver.
  *          This file provides functions to manage the SPI module  RHD2000:        
  @verbatim
*/

/*************************************************************************
              ##### How to use this driver #####
              
Example : N = 2 ==> 2 bits ==> 3 cuts values ==> 4 winners
						  CUT_VAL_SIZE = 3   ==> (2^2 - 1)

cut val i :							0					1					2 	
							|---------|---------|---------|---------|
winner :					00				01				10				11
Delta	 :			|---------|

EtaAdd  [0] = Eta/1			[1] =	Eta/2			[2] =	Eta/3		
EtaSous [0] = Eta/3			[1] =	Eta/2			[2] =	Eta/1	             
*************************************************************************/

#include "FBAR.h"

#define H_     120/128
#define BETA   8
#define ETA_   512
#define DELTA_ 400

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
static int16_t EtaAdd[CUT_VAL_SIZE]    = {0};
static int16_t EtaSous[CUT_VAL_SIZE]   = {0};
static int16_t PrevPrediction[CHANNEL_SIZE] = {0};

static int16_t cutValue[CHANNEL_SIZE][CUT_VAL_SIZE]     = {0}; 
static int16_t cutValueSave[CHANNEL_SIZE][CUT_VAL_SIZE] = {0}; 
static int16_t Delta           = 0; // initial resolution compression = range / number of cut value
static int16_t PredictorError  = 0;
static int16_t Eta             = 0;
static int16_t Beta            = 1;

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
	int16_t i;
  
	if (EtaIndex < 100)
    EtaIndex = 100; 
  
  Eta = ETA_; //(EtaIndex - 100) * 10;
	
	Delta = DELTA_;
	
	// initialize the first cutvalues
	for(i=0; i < CHANNEL_SIZE; i++) 
	{
    PrevPrediction[i] = 0;
    
    cutValue[i][0] = - DELTA_;
    cutValue[i][1] = 0;
    cutValue[i][2] = DELTA_;
    
    cutValueSave[i][0] = - DELTA_;
    cutValueSave[i][1] = 0;
    cutValueSave[i][2] = DELTA_;
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
	static int16_t ValueCurrentChannel;
	static uint16_t i, winner;
  static uint16_t tmpValue;
  
	#pragma unroll_completely
	for(i=0; i < CHANNEL_SIZE; i++)
	{		
		tmpValue = *bufferFrom++;
    
    tmpValue = (tmpValue >> 1) &  0x7FFF;
    
    ValueCurrentChannel = (int16_t)tmpValue;
    
    PredictorError = ValueCurrentChannel - PrevPrediction[i];
    
    winner = FBAR_AdaptCutValues(i, PredictorError);
    
    *bufferTo++ = winner; 
    
    if(winner == CUT_VAL_SIZE)
    {
      if (PrevPrediction[i] + cutValue[i][CUT_VAL_SIZE-1] < 32767)
        PrevPrediction[i] = ( PrevPrediction[i] + cutValue[i][CUT_VAL_SIZE-1]); 
    }
    else if (!winner)
    {
      if (PrevPrediction[i] + cutValue[i][CUT_VAL_SIZE-1] > - 32767)
        PrevPrediction[i] = ( PrevPrediction[i] + cutValue[i][0]);
    }
    else
    {
      PrevPrediction[i] = ( PrevPrediction[i] + (cutValue[i][winner-1] + cutValue[i][winner]) /2);
    }  
    
    PrevPrediction[i] = PrevPrediction[i] * H_;
	}
}

/**************************************************************************/
//					FBAR_AdaptCutValues
/**************************************************************************/
static uint8_t FBAR_AdaptCutValues(uint8_t channel, int16_t Value)
{
  static uint8_t i, winner;  
  static int16_t TmpCut; 
  winner = 0;
  
  #pragma unroll_completely 
  for (i=0; i < CUT_VAL_SIZE; i++)
  {
    if (Value >= cutValue[channel][i])
    {
      TmpCut = EtaAdd[i]-(cutValue[channel][i]) / BETA;
      if(i == (CUT_VAL_SIZE-1))
      {        
        // on controle que les cutvalues ne d�passent pas l2^15 - 1 
        if (cutValue[channel][CUT_VAL_SIZE-1] < 32767 - TmpCut) 
          cutValue[channel][CUT_VAL_SIZE-1] += TmpCut;
      }
      // anti chevauchement (et compilation warning i+1, depassement range tableau, secu � pr�voir)
      else 
      {  
        if ((cutValue[channel][i+1] - cutValue[channel][i]) >= TmpCut) 
          cutValue[channel][i] += TmpCut;	
      }
      winner = i+1; 
    }
    else	
    {
      TmpCut = -EtaSous[i]-(cutValue[channel][i]) / BETA;
      if (!i)
      {
        // on controle que les cutvalues ne d�passent pas 0 en n�gatif
        if (cutValue[channel][0] > (-32767) + TmpCut)  
          cutValue[channel][0] += TmpCut;        
      }
      // anti chevauchement (et compilation warning i-1, depassement negatif range tableau, secu � pr�voir)
      else 
      {
        if (((cutValue[channel][i] - cutValue[channel][i-1]) >= TmpCut))  
          cutValue[channel][i] +=  TmpCut;	
      }
    }
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



