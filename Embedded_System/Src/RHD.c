#include "RHD.h"

uint8_t decal = 0;

//uint16_t testValue[SIZE_VALUE] = { 4300,  8600, 11000,  8400, 12000, 
//																	 9000, 14000, 18000, 24000, 28000, 
//																	24000, 18000, 15000, 11000, 16000, 
//																	10000,  9000,  8000,  9000,  8000,
//																	50000,  8000, 50000,  8000,  8000};


uint16_t testValue[SIZE_VALUE] = {10000, 20000, 30000, 40000, 50000, 
																	40000, 30000, 20000, 10000, 00000,
																	10000, 20000, 30000, 40000, 50000,
																	40000, 30000, 20000, 10000, 00000,
																	10000, 10000, 10000, 10000, 00000};

uint16_t testBuffer[SIZE_TEST];

static uint16_t channel[CHANNEL_SIZE] = {(MASK_CONVERT | CHANNEL0), 
																				 (MASK_CONVERT | CHANNEL1), 
																				 (MASK_CONVERT | CHANNEL2), 
																				 (MASK_CONVERT | CHANNEL3), 
																				 (MASK_CONVERT | CHANNEL4), 
																				 (MASK_CONVERT | CHANNEL5), 
																				 (MASK_CONVERT | CHANNEL6), 
																				 (MASK_CONVERT | CHANNEL7)};
																				 
//static uint16_t channelTest[CHANNEL_SIZE] = {(0xE800),(0xE900),(0xEA00),(0xEB00),(0xEC00),(0xEC00),(0xEC00),(0xEC00)};  // read "INTANNN"																				
																				 
static void InitTestBuffer(void);																				 
																				 
// **************************************************************
// 	 				RHD_Init 
// **************************************************************
void RHD_Init(void) 
{
	GPIOInit();
  Spi2Init();
	RegisterInit();
	InitTestBuffer();
}

// **************************************************************
//					GPIOInit 
// **************************************************************
static void GPIOInit(void) 
{
	//init structures for the config
  GPIO_InitTypeDef GPIO_InitStructure;
	
	// enable clock for PORT F and C
	__GPIOF_CLK_ENABLE();
	__GPIOB_CLK_ENABLE();
	
	// configure pins used by SPI2
	// PB13 = SCK
	// PB14  = MISO
	// PB15  = MOSI
	GPIO_InitStructure.Pin 			 = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	GPIO_InitStructure.Mode 		 = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed 		 = GPIO_SPEED_MEDIUM;
	GPIO_InitStructure.Pull 		 = GPIO_NOPULL;
	GPIO_InitStructure.Alternate = GPIO_AF0_SPI2; 
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
		
	// Configure the chip SELECT pin 
	// PF7 = CSN
	GPIO_InitStructure.Pin 		= GPIO_PIN_7;
	GPIO_InitStructure.Mode 	= GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);	
	GPIOF->BSRR |= (uint32_t) GPIO_PIN_7; // set PF7 HIGH	
}		

static SPI_HandleTypeDef SpiHandle;
// **************************************************************
//					SpiInit
// **************************************************************
static void Spi2Init(void)
{	
	// enable SPI2 clock
	__SPI2_CLK_ENABLE();
	
	// Set the SPI parameters 
  SpiHandle.Instance               = SPI2;
  SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
  SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
  SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
  SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
  SpiHandle.Init.CRCPolynomial     = 7;
  SpiHandle.Init.DataSize          = SPI_DATASIZE_16BIT;
  SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  SpiHandle.Init.NSS               = SPI_NSS_SOFT;
  SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLED;
  SpiHandle.Init.NSSPMode          = SPI_NSS_PULSE_DISABLED;
  SpiHandle.Init.CRCLength         = SPI_CRC_LENGTH_8BIT;
	SpiHandle.Init.Mode 						 = SPI_MODE_MASTER;
	HAL_SPI_Init(&SpiHandle);

  // enable SPI2 global interrupt
  HAL_NVIC_SetPriority(SPI2_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(SPI2_IRQn);	

	//__HAL_SPI_ENABLE_IT(&SpiHandle, SPI_IT_RXNE | SPI_IT_TXE);

	__HAL_SPI_ENABLE(&SpiHandle);
}

static uint8_t  channelIndex = 0;
static uint16_t * bufferSample;
/**************************************************************/
//					SPI2_IRQHandler
/**************************************************************/
void SPI2_IRQHandler()
{
	// If RX buffer is NOT empty then write it to the buffer
	if (SPI2->SR & SPI_FLAG_RXNE)
	{
			//*bufferSample++ = (((SPI2->DR) >> decal) & 0x00FF ); // send SPI data to the buffer
			*bufferSample++ = (SPI2->DR) >> 1;
	}
	// If SPI is not busy then we have finished sending data
	if (!(SPI2->SR & SPI_FLAG_BSY))
	{
		CsnDigitalWrite(HIGH); // CsnDigitalWrite(HIGH)
		// if some channel remain not sampled
		if (channelIndex < CHANNEL_SIZE)	
		{
			__nop(); // small delay for the slave select minimum waiting time	
			CsnDigitalWrite(LOW); // CsnDigitalWrite(LOW)
			SPI2->DR = channel[channelIndex++];
		}
		else	// if all the channel have been sampled
		{
			channelIndex = 0;
			__HAL_SPI_DISABLE_IT(&SpiHandle, SPI_IT_RXNE | SPI_IT_TXE);
		}	
	}
	SPI2->SR &=  ~(SPI_IT_RXNE | SPI_IT_TXE); // clear flag to don't trigger directly an other interrupt
}

/**************************************************************/
//					Spi2ReturnSend
/**************************************************************/
uint16_t	Spi2ReturnSend(uint16_t data)
{
	CsnDigitalWrite(LOW);
	
	SPI2->DR = data; 	// write data to be transmitted to the SPI data register
	while( !(SPI2->SR & SPI_FLAG_TXE)  ); // wait until transmit complete
	while( !(SPI2->SR & SPI_FLAG_RXNE) ); // wait until receive complete
	while(   SPI2->SR & SPI_FLAG_BSY   ); // wait until SPI is not busy anymore
	
	CsnDigitalWrite(HIGH);
	
	return SPI2->DR;
}

/**************************************************************/
// 					CsnDigitalWrite 
/**************************************************************/
static void CsnDigitalWrite(uint8_t state)
{
	if (state)  GPIOF->BSRR |= GPIO_PIN_7;
	else 				GPIOF->BSRR |= (uint32_t) (GPIO_PIN_7 << 16); 
}

/**************************************************************/
// 					RegisterInit 
/**************************************************************/
static void RegisterInit(void)
{
	volatile int i = 0;
	uint16_t reg[18] = {0};
	uint16_t calib = 0;
	uint16_t dummy = MASK_READ | 0x2C00 ;
		
	// Start all our configurations for RHD2000 board.
	// To write data D in register R we will sned the following half-word through SPI2 :
	// MASK_WRITE & (RHD2000_R"X" | DATA_R"X"));  
	// where "X" is the number of the register
	// example : (MASK_WRITE | (RHD2000_R4 | DATA_R4))
	// 				= (0x8000 | (	0x0400 | 0x009F))
	// 				= 0x849F = 0b1000 0100 1001 1111	
	
	// R0: ADC Configuration and Amplifier Fast Settle
	reg[0] = (MASK_WRITE | (RHD2000_R0 | DATA_R0) );	
	// R1: Supply sensor and ADC Buffer Bias Current
	reg[1] = (MASK_WRITE | (RHD2000_R1 | DATA_R1) ); 
	// R2: MUX Bias Current
	reg[2] = (MASK_WRITE | (RHD2000_R2 | DATA_R2) ); 
	// R3: MUX Load, Temperature Sensor, and Auxiliary Digital Output
	reg[3] = (MASK_WRITE | (RHD2000_R3 | DATA_R3) ); 
	// R4: ADC Output Format and DSP Offset Removal
	reg[4] = (MASK_WRITE | (RHD2000_R4 | DATA_R4) ); 
	// R5: Impedance Check Control
	reg[5] = (MASK_WRITE | (RHD2000_R5 | DATA_R5) ); 	
	// R6: Impedance Check DAC
	reg[6] = (MASK_WRITE | (RHD2000_R6 | DATA_R6) ); 
	// R7: Impedance Check Amplifier Select
	reg[7] = (MASK_WRITE | (RHD2000_R7 | DATA_R7) ); 
	// R8-11: On-Chip Amplifier Bandwith Select upper frequency
	reg[8]  = (MASK_WRITE | (RHD2000_R8 	| DATA_R8 ) ); 
	reg[9]  = (MASK_WRITE | (RHD2000_R9 	| DATA_R9 ) ); 
	reg[10] = (MASK_WRITE | (RHD2000_R10  | DATA_R10) );
	reg[11] = (MASK_WRITE | (RHD2000_R11  | DATA_R11) ); 
	// R12-13: On-Chip Amplifier Bandwith Select lower frequency
	reg[12] = (MASK_WRITE | (RHD2000_R12 | DATA_R12) );
	reg[13] = (MASK_WRITE | (RHD2000_R13 | DATA_R13) ); 
	// R14-17: Individual Amplifier Power select the input used in RHD2000
	reg[14] = (MASK_WRITE | (RHD2000_R14 | DATA_R14) ); 
	reg[15] = (MASK_WRITE | (RHD2000_R15 | DATA_R15) ); 
	reg[16] = (MASK_WRITE | (RHD2000_R16 | DATA_R16) ); 
	reg[17] = (MASK_WRITE | (RHD2000_R17 | DATA_R17) ); 

	for(i = 0; i<sizeof(reg); i++)	{Spi2ReturnSend(reg[i]);}
	for(i = 0; i < 50; i++)	{__nop();}	// small delay
	
	// Calibrate the RHD2000 (automatic routine)
	calib = (MASK_CALIBRATE); 
	Spi2ReturnSend(calib);
	
	for(i = 0; i < 9; i++)	{Spi2ReturnSend(dummy);} // send dummy bytes for the calibration's waiting time
	Spi2ReturnSend(dummy);
	for(i = 0; i < 50; i++)	{__nop();}				// small delay	
}

/**************************************************************/
//	 				RHD_Sample 
/**************************************************************/
void RHD_Sample(uint16_t * buffer)
{
	bufferSample = buffer;
	
	// enable interrupt : the sampling is done in the interrupt handler
	__HAL_SPI_ENABLE_IT(&SpiHandle, SPI_IT_RXNE | SPI_IT_TXE);
}

/**************************************************************/
//	 				InitTestBuffer
/**************************************************************/
static void InitTestBuffer(void)
{
	uint16_t i, j, iPlusOne, delta;
	
	for(i = 0; i < SIZE_VALUE; i++)
	{	
		if (i == SIZE_VALUE - 1)	iPlusOne = 0;
		else											iPlusOne = i+1;
	
		delta = (testValue[iPlusOne] - testValue[i]) / INTERVAL_TEST; //either positive or negative
		
		for(j = 0; j < INTERVAL_TEST; j++)		
		{
			testBuffer[(INTERVAL_TEST * i) + j] = testValue[i] + (j * delta);
		}
	}
}

static uint8_t chan = 0, time = 0, freq = 0, freq2 = 0;		
static uint16_t *bufferSampleTest;
static uint16_t testElement = 0;
/**************************************************************/
//	 				RHD_SampleTest
/**************************************************************/
void RHD_SampleTest(uint16_t * buffer, uint8_t test)
{
	bufferSampleTest = buffer;	
	
	switch (test) 
	{
		case 1 :
		{
			for (chan = 0; chan < CHANNEL_SIZE; chan++)
				*bufferSampleTest++ = testBuffer[testElement];
			
			testElement++;
			
			if (testElement >= SIZE_TEST)
				testElement = 0;
			
			break;
		}
		
		case 2 :
		{
			for (chan = 0; chan < CHANNEL_SIZE; chan++)
				*bufferSampleTest++ = time;
			time++;
			if (time > 110) 
				time = 0; 
			break;
		}
				
		case 3 :
		{	
			for (chan = 0; chan < CHANNEL_SIZE; chan++)
			{
				if((chan + 1) % 2)
					//*bufferSampleTest++ = ((time + 1) * (chan + 1));
					*bufferSampleTest++ = time;
				else 
					*bufferSampleTest++ = (chan + 1) * 10;
			}
			time++;
			if (time > freq + 10) 
			{
				time=1;	
				freq2++;
				if (freq2 >10)
				{
					freq+=10;
					freq2 = 1;
				}
			}
			if (freq>100) 
				freq = 0;
			break;
		}
		
		default :
			break;	
	}		
}	


