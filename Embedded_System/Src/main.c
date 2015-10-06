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
static DataStateTypeDef DataState = FIRST_STATE;
static uint8_t EtaIndex;

// *************************************************************************
// *************************************************************************
// 						Extern variables	
// *************************************************************************
// *************************************************************************
extern volatile uint16_t Fbar_Eta;
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
  
  if (DataState != NRF_GetDataState())
  {
    DataState = NRF_GetDataState();
    EtaIndex = NRF_GetEtaIndex();
    SampleSend_SetState(DataState);
    DataBuffer_ChangeState(DataState, (uint8_t) EtaIndex);
    SampleSend_SetState(DataState);
  }
  
  if ((DataState == __8ch_3bit__20kHz__C__ ) && (EtaIndex != NRF_GetEtaIndex()))
  {
    EtaIndex = NRF_GetEtaIndex();
    DataBuffer_ChangeState(__8ch_3bit__20kHz__C__, (uint8_t) EtaIndex);
    SampleSend_SetState(__8ch_3bit__20kHz__C__);
  }   
  SampleSend_Enable(HIGH);
}














