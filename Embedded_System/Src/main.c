#include "stm32f0xx_hal.h"
#include "NRF.h"
#include "SampleSend.h"
#include "board_interface.h"
#include "CommonDefine.h"

static void ChangeDataState(void);

volatile static DataStateTypeDef DataState = FIRST_STATE;

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
      ChangeDataState();  
  } 
}

static void ChangeDataState(void)
{
  SampleSend_Enable(LOW);
  
  DataState = NRF_GetDataState();
  DataBuffer_ChangeState(DataState);
  SampleSend_SetState(DataState);
  
  SampleSend_Enable(HIGH);
}














