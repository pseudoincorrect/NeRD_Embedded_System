#include "SampleSend.h"

uint8_t  SampleBuffer[2][SAMPLE_BUFFER_SIZE] = {0}; // Main sample buffer 
static uint8_t  SampleBufferTabRhdIndex = 0; // index of the current array used by the RHD (write)
static uint8_t  SampleBufferTabNrfIndex = 0; // index of the current array used by the NRF (read)

static uint16_t SampleBufferIndex 			= 0; // index where the RHD is writing in the main sample buffer
static uint8_t  SendEnable 						  = 0; // Flag to enable the sending of data (wirelessly)
uint8_t 			  SampleEnable 						= 0; // Flag to enable the sampling by the RHD

volatile uint32_t Tim1DIER;
volatile uint32_t Tim1SR;
	
static TIM_HandleTypeDef    TimHandle;

/**************************************************************/
//					SampleSend_Init
/**************************************************************/
void SampleSend_Init(void)
{ 
	NRF_Init();
	RHD_Init();
	TIM2Init(268, 8); // (268,8) =  20 kHz sample
}

/**************************************************************/
//					TIM2Init
/**************************************************************/
static void TIM2Init(uint32_t reloadValue, uint16_t prescalerValue)
{	
  __TIM2_CLK_ENABLE(); 

	TimHandle.Instance = TIM2;

	TimHandle.Init.Period            = reloadValue;
  TimHandle.Init.Prescaler         = prescalerValue;
  TimHandle.Init.ClockDivision     = 0;
  TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  TimHandle.Init.RepetitionCounter = 0;
	HAL_TIM_Base_Init(&TimHandle);
  
	// Set the TIMx priority 
	HAL_NVIC_SetPriority(TIM2_IRQn, 1, 0);	
	// Enable the TIMx global Interrupt 
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
	
	__HAL_TIM_DISABLE_IT(&TimHandle, TIM_IT_UPDATE); //The specified TIM3 interrupt : update
  __HAL_TIM_DISABLE(&TimHandle);
}

/**************************************************************/
//					TIM2_IRQHandler
/**************************************************************/
void TIM2_IRQHandler(void)
{	
	if(__HAL_TIM_GET_FLAG(&TimHandle, TIM_FLAG_UPDATE) != RESET)
	{
		if(__HAL_TIM_GET_ITSTATUS(&TimHandle, TIM_IT_UPDATE) !=RESET)
		{ 
			__HAL_TIM_CLEAR_IT(&TimHandle, TIM_IT_UPDATE); // Remove TIMx update interrupt flag 
			__HAL_TIM_CLEAR_FLAG(&TimHandle, TIM_IT_UPDATE);
		}
		if ( (SAMPLE_BUFFER_SIZE - SampleBufferIndex) < CHANNEL_SIZE ) // if there is not enough room in SampleBuffer
		{
			SampleBufferTabRhdIndex = (!SampleBufferTabRhdIndex) ? 1 : 0; 
			SampleBufferIndex  = 0;		
		}
		 RHD_SampleTest(&(SampleBuffer[SampleBufferTabRhdIndex][SampleBufferIndex]), 1);
		//RHD_Sample( &(SampleBuffer[SampleBufferTabRhdIndex][SampleBufferIndex])); 
		SampleBufferIndex  += CHANNEL_SIZE;
		
		if (SampleBufferTabRhdIndex != SampleBufferTabNrfIndex)	{SendEnable = 1;}
	}
}	

/**************************************************************/
//					SampleSend_Enable
/**************************************************************/
void SampleSend_Enable(uint8_t state)
{		
		SampleEnable = state;	
		if(SampleEnable)
		{
			SendEnable = 0;
			SampleBufferTabRhdIndex = 0; SampleBufferTabNrfIndex = 0;	SampleBufferIndex = 1;
			__HAL_TIM_ENABLE_IT(&TimHandle, TIM_IT_UPDATE); // TIM2 interrupt : update
			__HAL_TIM_ENABLE(&TimHandle);
		}
		else
		{
			__HAL_TIM_DISABLE_IT(&TimHandle, TIM_IT_UPDATE); // TIM2 interrupt : update
			__HAL_TIM_DISABLE(&TimHandle);				
		}
}

/**************************************************************/
//					SampleSend_Acquisition
/**************************************************************/
void SampleSend_Acquisition(void)
{
	if (SendEnable)
	{	
		uint8_t IndexTmp;
		
		SendEnable = 0;
		
		IndexTmp = SampleBufferTabNrfIndex;
		SampleBufferTabNrfIndex = SampleBufferTabRhdIndex;
		
		NRF_SendBuffer(&SampleBuffer[IndexTmp][0]);
	}
}












