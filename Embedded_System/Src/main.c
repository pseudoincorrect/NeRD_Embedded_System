#include "stm32f0xx_hal.h"
#include "NRF.h"
#include "SampleSend.h"
#include "board_interface.h"
#include "CommonDefine.h"

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
  } 
}


