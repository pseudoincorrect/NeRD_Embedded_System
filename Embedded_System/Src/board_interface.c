#include "board_interface.h"

//static Board_StateTypeDef BoardState = IDDLE;

/**************************************************************/
//					SystemClock_Config
/**************************************************************/
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

  __SYSCFG_CLK_ENABLE();
}

/**************************************************************/
//					HAL_MspInit
/**************************************************************/
void HAL_MspInit(void)
{
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/**************************************************************/
//					SysTick_Handler
/**************************************************************/
void SysTick_Handler(void)
{
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
}

/**************************************************************/
//					Board_Init
/**************************************************************/
void Board_Init(void)
{
	GpioInit();
  TIMInit(1000,100);
}

/**************************************************************/
//					GpioInit
/**************************************************************/
static void GpioInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

  /* TIM3 and GPIO Ports Clock Enable */
  __TIM3_CLK_ENABLE();
  __GPIOA_CLK_ENABLE();
  __GPIOB_CLK_ENABLE();

	//******************* LED Pin

	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Pin   =  GPIO_PIN_4;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  GPIOB->BSRR |= (uint32_t) (GPIO_PIN_4 << 16); // set PB_4 LOW

  //******************* Debug pin (USR)
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  DEBUG_LOW;
}

static TIM_HandleTypeDef    TimHandle;
/**************************************************************/
//					TIM3Init
/**************************************************************/
static void TIMInit(uint32_t reloadValue, uint16_t prescalerValue)
{	
	TimHandle.Instance = TIM3;

	TimHandle.Init.Period            = reloadValue;
  TimHandle.Init.Prescaler         = prescalerValue;
  TimHandle.Init.ClockDivision     = 0;
  TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  TimHandle.Init.RepetitionCounter = 0;
 	HAL_TIM_Base_Init(&TimHandle);
  
	// Set the TIMx priority 
	HAL_NVIC_SetPriority(TIM3_IRQn, 2, 0);	
	// Enable the TIMx global Interrupt 
  HAL_NVIC_DisableIRQ(TIM3_IRQn);
	
	__HAL_TIM_ENABLE_IT(&TimHandle, TIM_IT_UPDATE); //The specified TIM3 interrupt : update
  __HAL_TIM_ENABLE(&TimHandle);
}

uint32_t Tim3Ticks = 0;
/**************************************************************/
//					TIM3_IRQHandler
/**************************************************************/
void TIM3_IRQHandler(void)
{	
	if(__HAL_TIM_GET_FLAG(&TimHandle, TIM_FLAG_UPDATE) != RESET)
	{
		if(__HAL_TIM_GET_ITSTATUS(&TimHandle, TIM_IT_UPDATE) !=RESET)
		{
      Tim3Ticks++;
      if(Tim3Ticks > 10)
      {
        Tim3Ticks = 0;
        GPIOB->BSRR |= (uint32_t) (GPIO_PIN_4 << 16); // set PB_4 LOW
        HAL_NVIC_DisableIRQ(TIM3_IRQn);
      }
			__HAL_TIM_CLEAR_IT(&TimHandle, TIM_IT_UPDATE); // Remove TIMx update interrupt flag 
			__HAL_TIM_CLEAR_FLAG(&TimHandle, TIM_IT_UPDATE);
		}	
	}
}

/**************************************************************/
//					Board_LedPulse
/**************************************************************/
void Board_LedPulse(void)
{
	GPIOB->BSRR |= (uint32_t) GPIO_PIN_4; // set PB_4 HIGH
  HAL_NVIC_EnableIRQ(TIM3_IRQn);
}















