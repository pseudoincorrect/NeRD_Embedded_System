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
#define BETA   BETA_INIT
#define ETA_   ETA_INIT
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
static int16_t EtaAdd[CUT_VAL_SIZE]                 = {0};
static int16_t EtaSous[CUT_VAL_SIZE]                = {0};
static int16_t PrevPrediction[CHANNEL_SIZE]         = {0};
static int16_t cutValue[CHANNEL_SIZE][CUT_VAL_SIZE] = {0}; 
static int16_t PredictorError  = 0;
static int16_t Eta, Beta       = 0;

//*************************************************************************
//*************************************************************************
// 										Function definitions																 
//*************************************************************************
//*************************************************************************
/**************************************************************************/
//					FBAR_Initialize
/**************************************************************************/
void FBAR_Initialize(uint16_t EtaParam, uint16_t BetaParam)
{
	int16_t i;

  Eta  = EtaParam;
  if (BetaParam > 0 && BetaParam <= 128)
    Beta  = BetaParam;
  
		// initialize the first cutvalues
	for(i=0; i < CHANNEL_SIZE; i++) 
	{
    PrevPrediction[i] = 0;
    
    cutValue[i][0] = - DELTA_;
    cutValue[i][1] = 0;
    cutValue[i][2] = DELTA_;
  }
  
  for (i=0; i < CUT_VAL_SIZE; i++)
	{
		EtaSous[i] = Eta / (i + 1);
		EtaAdd[i]  = Eta / (CUT_VAL_SIZE - i);
	} 
}

/**************************************************************************/
//					FBAR_Reinitialize
/**************************************************************************
/ this function will reinialise the cutvalues from the current value 
/ for each channel and will send the current channel's values 
/ to the base system by creating a frame comporting  a balise 
/ (first two bytes = 0xFF and then the values of every channel on 16 bits) 
/ before being sent to the NRF
**************************************************************************/
void FBAR_Reinitialize(uint8_t * bufferTo1, uint8_t * bufferTo2, uint8_t * bufferTo3)
{
	static volatile uint16_t i, j, valueFrom;
  
	*bufferTo1++ = 0xFF;	*bufferTo1++ = 0xFF;  *bufferTo1++ = 0x01;
  
  #pragma unroll_completely
	for(i=0; i < CHANNEL_SIZE; i++)
	{	
    valueFrom = PrevPrediction[i];
    *bufferTo1++ = (valueFrom >> 8);
		*bufferTo1++ = (valueFrom & 0xFF);
	}
  
  *bufferTo2++ = 0xFF;	*bufferTo2++ = 0xFF;  *bufferTo2++ = 0x02;
  
  #pragma unroll_completely
	for(i=0; i < CHANNEL_SIZE/2; i++)
	{		
    #pragma unroll_completely
    for(j=0; j < CUT_VAL_SIZE; j++)
    {
      *bufferTo2++ = (cutValue[i][j] >> 8);
      *bufferTo2++ = (cutValue[i][j] & 0xFF);
    }
	}
  
  *bufferTo3++ = 0xFF;	*bufferTo3++ = 0xFF;   *bufferTo3++ = 0x03;
  
  #pragma unroll_completely
	for(i=CHANNEL_SIZE/2; i < CHANNEL_SIZE; i++)
	{		
		#pragma unroll_completely
    for(j=0; j < CUT_VAL_SIZE; j++)
    {
      *bufferTo3++ = (cutValue[i][j] >> 8);
      *bufferTo3++ = (cutValue[i][j] & 0xFF);
    }
	}
} 

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
    
    tmpValue = (tmpValue >> 1) & 0x7FFF;
    
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
      //TmpCut = EtaAdd[i]-(cutValue[channel][i]) / Beta;
      switch (Beta)
      {
        case (1)  : TmpCut = EtaAdd[i]-(cutValue[channel][i]) / 1  ; break;
        case (2)  : TmpCut = EtaAdd[i]-(cutValue[channel][i]) / 2  ; break;
        case (4)  : TmpCut = EtaAdd[i]-(cutValue[channel][i]) / 4  ; break;
        case (8)  : TmpCut = EtaAdd[i]-(cutValue[channel][i]) / 8  ; break;
        case (16) : TmpCut = EtaAdd[i]-(cutValue[channel][i]) / 16 ; break;
        case (32) : TmpCut = EtaAdd[i]-(cutValue[channel][i]) / 32 ; break;                    
        case (64) : TmpCut = EtaAdd[i]-(cutValue[channel][i]) / 64 ; break; 
        case (128): TmpCut = EtaAdd[i]-(cutValue[channel][i]) / 128; break; 
        default   : TmpCut = EtaAdd[i]-(cutValue[channel][i]) / 128; break;
      }      
      
      if(i == (CUT_VAL_SIZE-1))
      {        
        // on controle que les cutvalues ne dépassent pas l2^15 - 1 
        if (cutValue[channel][CUT_VAL_SIZE-1] < 32767 - TmpCut) 
          cutValue[channel][CUT_VAL_SIZE-1] += TmpCut;
      }
      // anti chevauchement (et compilation warning i+1, depassement range tableau, secu à prévoir)
      else 
      {  
        if ((cutValue[channel][i+1] - cutValue[channel][i]) >= TmpCut) 
          cutValue[channel][i] += TmpCut;	
      }
      winner = i+1; 
    }
    else	
    {
      //TmpCut = -EtaSous[i]-(cutValue[channel][i]) / Beta;
      switch (Beta)
      {
        case (1)  : TmpCut = -EtaSous[i]-(cutValue[channel][i]) / 1  ; break;
        case (2)  : TmpCut = -EtaSous[i]-(cutValue[channel][i]) / 2  ; break;
        case (4)  : TmpCut = -EtaSous[i]-(cutValue[channel][i]) / 4  ; break;
        case (8)  : TmpCut = -EtaSous[i]-(cutValue[channel][i]) / 8  ; break;
        case (16) : TmpCut = -EtaSous[i]-(cutValue[channel][i]) / 16 ; break;
        case (32) : TmpCut = -EtaSous[i]-(cutValue[channel][i]) / 32 ; break;                    
        case (64) : TmpCut = -EtaSous[i]-(cutValue[channel][i]) / 64 ; break; 
        case (128): TmpCut = -EtaSous[i]-(cutValue[channel][i]) / 128; break; 
        default :   TmpCut = -EtaSous[i]-(cutValue[channel][i]) / 128; break;
      } 
       
      if (!i)
      {
        // on controle que les cutvalues ne dépassent pas 0 en négatif
        if (cutValue[channel][0] > (-32767) + TmpCut)  
          cutValue[channel][0] += TmpCut;        
      }
      // anti chevauchement (et compilation warning i-1, depassement negatif range tableau, secu à prévoir)
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



