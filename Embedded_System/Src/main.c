// *************************************************************************
/* ************************************************************************
  * @file    main.c
  * @author  Maxime CLEMENT
  * @version V1.0
  * @date    06-Oct-2015
  * @brief   RHD2000 module driver driver.
  *          This file provides functions to manage the SPI module  RHD2000:        
  @verbatim
*/

#include "stm32f0xx_hal.h"
#include "NRF.h"
#include "SampleSend.h"
#include "board_interface.h"
#include "CommonDefine.h"

// *************************************************************************
// *************************************************************************
// 						Static functions	
// *************************************************************************
// *************************************************************************
static void ChangeDataState(void);

// *************************************************************************
// *************************************************************************
// 						Static variables	
// *************************************************************************
// *************************************************************************
static DataStateTypeDef DataState = STATE_INIT;
static uint16_t Eta = ETA_INIT, Beta = BETA_INIT;

// *************************************************************************
// *************************************************************************
// 						Extern variables	
// *************************************************************************
// *************************************************************************
extern DataStateTypeDef DataStateNRF;

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
  
  DataState = NRF_GetDataState();
  
  if (DataState == __8ch_2bit__20kHz__C__ )
  {
    Eta = NRF_GetEta();
    Beta = NRF_GetBeta();
  }
  
  DataBuffer_ChangeState(DataState, Eta, Beta);
  
  SampleSend_SetState(DataState);

  SampleSend_Enable(HIGH);
}













