#include "stm32f0xx_hal.h"
#include "NRF.h"
#include "SampleSend.h"
#include "board_interface.h"
#include "CommonDefine.h"

static void ChangeDataState(void);

static DataStateTypeDef DataState = FIRST_STATE;

extern DataStateTypeDef DataStateNRF;
  
static uint8_t EtaIndex;

extern volatile uint16_t Fbar_Eta;

int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  Board_Init();
 
	/* Initialize the NRF moduke */
	SampleSend_Init();
	
	SampleSend_Enable(HIGH);
  
	while (1)
  {
		SampleSend_Acquisition();
		DataBuffer_Process();
    if(NRF_CheckChange()) 
    {
      Board_LedPulse();
      ChangeDataState();
    }
  } 
}

static void ChangeDataState(void)
{
  SampleSend_Enable(LOW);
  
  if (DataState != NRF_GetDataState())
  {
    DataState = NRF_GetDataState();
    EtaIndex = NRF_GetEtaIndex();
    SampleSend_SetState(DataState);
    DataBuffer_ChangeState(DataState, (uint8_t) EtaIndex);
    SampleSend_SetState(DataState);
  }
  
  if ((DataState == __8ch_16bit_20kHz__C__ ) && (EtaIndex != NRF_GetEtaIndex()))
  {
    EtaIndex = NRF_GetEtaIndex();
    DataBuffer_ChangeState(__8ch_16bit_20kHz__C__, (uint8_t) EtaIndex);
    SampleSend_SetState(__8ch_16bit_20kHz__C__);
  }   
  SampleSend_Enable(HIGH);
}














