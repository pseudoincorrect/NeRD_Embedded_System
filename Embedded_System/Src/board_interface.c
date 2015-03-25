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
	PwmInit();
	Board_Leds(IDDLE);
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

	
	 /* Configure PA.15 pin as input floating */
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Enable and set EXTI line 0 Interrupt to the lowest priority */
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
	
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;

	GPIO_InitStruct.Alternate = GPIO_AF1_TIM3;
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.Alternate = GPIO_AF1_TIM3;
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
}

extern uint8_t decal; // Décalage à bariller des donnes de la RHD
uint8_t toggle = 1;
/**************************************************************/
//					EXTI15_10_IRQHandler
/**************************************************************/
void EXTI4_15_IRQHandler(void)
{
	if (EXTI->PR & EXTI_PR_PR15)
	{
		decal++;
		if (decal > 8)
			decal = 0;
		
		if (!decal)
			Board_Leds(IDDLE);	
		else
			Board_Leds(TRANSMITTING);
		
		/*if (toggle)	
		{		
			toggle = !toggle;
			BoardState = TRANSMITTING;
			Board_Leds(BoardState);			
			SampleSend_Enable(HIGH);
		}
		else
		{
			toggle = !toggle;			
			BoardState = IDDLE;
			Board_Leds(BoardState);			
			SampleSend_Enable(LOW);			
		}
		*/
	}
	EXTI->PR = EXTI_PR_PR15;
}


TIM_HandleTypeDef    TimHandle;
TIM_OC_InitTypeDef 	 sConfig;
/**************************************************************/
//					PwmInit
/**************************************************************/
static void PwmInit(void)
{	
	TimHandle.Instance = TIM3;

  TimHandle.Init.Prescaler         = 0;
  TimHandle.Init.Period            = 1000;
  TimHandle.Init.ClockDivision     = 0;
  TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  TimHandle.Init.RepetitionCounter = 0;
  HAL_TIM_PWM_Init(&TimHandle);

  /*##-2- Configure the PWM channels #########################################*/
  /* Common configuration for all channels */
  sConfig.OCMode       = TIM_OCMODE_PWM1;
  sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
  sConfig.OCFastMode   = TIM_OCFAST_DISABLE;
  sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
  sConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;
  sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;

	sConfig.Pulse = 1000/10;
  HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, TIM_CHANNEL_1);
	
	sConfig.Pulse = 1000/10;
  HAL_TIM_PWM_ConfigChannel(&TimHandle, &sConfig, TIM_CHANNEL_2);
	
	HAL_TIM_PWM_Start(&TimHandle, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&TimHandle, TIM_CHANNEL_2);
}

/**************************************************************/
//					Board_RedLed
/**************************************************************/	
static void RedLed(uint8_t state)
{
	if (state)
		HAL_TIM_PWM_Start(&TimHandle, TIM_CHANNEL_2);
	else
		HAL_TIM_PWM_Stop(&TimHandle, TIM_CHANNEL_2);
}

/**************************************************************/
//					Board_GreellowLed
/**************************************************************/
static void GreellowLed(uint8_t state)
{
	if (state)
		HAL_TIM_PWM_Start(&TimHandle, TIM_CHANNEL_1);
	else
		HAL_TIM_PWM_Stop(&TimHandle, TIM_CHANNEL_1);
}

/**************************************************************/
//					Board_Leds
/**************************************************************/
static void Board_Leds(Board_StateTypeDef state)
{
	switch(state)
	{
		case IDDLE :
			GreellowLed(LOW);
			RedLed(HIGH);
			break;
		case TRANSMITTING :
			GreellowLed(HIGH);
			RedLed(LOW);
			break;
		case SETTING :
			GreellowLed(HIGH);
			RedLed(LOW);
			break;
		case STOP :
			GreellowLed(LOW);
			RedLed(LOW);
			break;
	}		
}

















