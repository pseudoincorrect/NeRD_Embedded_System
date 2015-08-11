#include "NRF.h"

// *************************************************************************
// *************************************************************************
// 						static variables	
// *************************************************************************
// *************************************************************************
static uint8_t RxAdressP0[6] 			= {W_REGISTER | RX_ADDR_P0, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7};  // set RX ADDR P0
static uint8_t TxAdress[6]  			= {W_REGISTER | TX_ADDR 	, 0xE7, 0xE7, 0xE7, 0xE7, 0xE7}; // set RX ADDR P0
static uint8_t DisableAutoAck[2] 	= {W_REGISTER | EN_AA 		, 0x00};         // disable auto acknowledgement
static uint8_t RxPayloadSize[2] 	= {W_REGISTER | RX_PW_P0 	, 32  };        // set size payload 32 byte
static uint8_t RxPipe[2] 					= {W_REGISTER | EN_RXADDR , 0x01};       // set pipe enabled (pipe 0)   0b 0000 0001 
static uint8_t RxTxAdressSize[2] 	= {W_REGISTER | SETUP_AW  , 0x01};      // set size adresse RX 3 byte   0b 0000 0001
static uint8_t RfChanel[2] 				= {W_REGISTER | RF_CH 		, 0x02};     // set RF chanel (2.4GHz+2MHz)	  0b 0000 0010  
static uint8_t RfParameter[2] 		= {W_REGISTER | RF_SETUP  , 0x0E};    // set RF parameters (2Mbps, 0dB) 0b 0000 1110
static uint8_t ClearIrqFlag[2]    = {W_REGISTER | STATUS    , 0x70}; // clear IRQ                         0b 1110 0000

//config without CRC
//static uint8_t ReceiveMode[2] 		= {W_REGISTER | CONFIG		, 0x33};   // set Receive mode  0b 0011 0011 
//static uint8_t TransmitMode[2]    = {W_REGISTER | CONFIG    , 0x52};  // set Transmit mode  0b 0101 0010
//config with CRC 1 byte
//static uint8_t ReceiveMode[2] 	= {W_REGISTER | CONFIG		, 0x3B};   // set Receive mode  0b 0011 1011 
//static uint8_t TransmitMode[2] = {W_REGISTER | CONFIG    , 0x5A};  // set Transmit mode  0b 0101 1010
//config with CRC 2 bytes
static uint8_t ReceiveMode[2] 	= {W_REGISTER | CONFIG		, 0x3F};   // set Receive mode    0b 0011 1111 
static uint8_t TransmitMode[2] = {W_REGISTER | CONFIG    , 0x5E};  // set Transmit mode    0b 0101 1110

static uint8_t FlushRxFifo 				= FLUSH_RX;                       // flush Rx fifo
static uint8_t FlushTxFifo				= FLUSH_TX;                      // flush Tx fifo

DataStateTypeDef DataStateNRF = FIRST_STATE;
static uint8_t EtaIndex;
static uint8_t DataStateFLAG = 0;

// *************************************************************************
// *************************************************************************
// 										Function definitions																 
// *************************************************************************
// *************************************************************************	
// **************************************************************
// 	 				NRF_Init 
// **************************************************************
void NRF_Init(void) 
{
	GPIOInit();
	SpiDmaInit(); 
	RegisterInit();
}

// **************************************************************
//					GPIOInit 
// **************************************************************
static void GPIOInit(void) 
{
	//init structures for the config
  GPIO_InitTypeDef GPIO_InitStructure;
	
	// enable clock for PORT A and C
	__GPIOA_CLK_ENABLE();
	__GPIOB_CLK_ENABLE();
	
	// configure pins used by SPI1
	// PA5 = SCK
	// PA6 = MISO
	// PA7 = MOSI
	GPIO_InitStructure.Pin 			 = GPIO_PIN_7 | GPIO_PIN_6 | GPIO_PIN_5;
	GPIO_InitStructure.Mode 		 = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Speed 		 = GPIO_SPEED_MEDIUM;
	GPIO_InitStructure.Pull 		 = GPIO_NOPULL;
	GPIO_InitStructure.Alternate = GPIO_AF0_SPI1; 
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
		
	// Configure the chip SELECT pin 
	// PB0 = CSN
	GPIO_InitStructure.Pin 		= GPIO_PIN_0;
	GPIO_InitStructure.Mode 	= GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);	
	GPIOB->BSRR |= (uint32_t) GPIO_PIN_0; // set PB_0 HIGH
	
	// Configure the chip ENABLE pin
	// PB1 = CE
	GPIO_InitStructure.Pin  = GPIO_PIN_1;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);	
	GPIOB->BSRR |= (uint32_t) (GPIO_PIN_1 << 16); // set PB_1 LOW
	
	// Configure the chip IRQ pin   
	// PA4 = IRQ
	GPIO_InitStructure.Pin   = GPIO_PIN_4;
  GPIO_InitStructure.Mode  = GPIO_MODE_INPUT;
  GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
  GPIO_InitStructure.Pull  = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
}

//init structures for the config 
static SPI_HandleTypeDef SpiHandle;
//init structures for the config 
static DMA_HandleTypeDef hdma_spi1;
// **************************************************************
//					SpiDmaInit
// **************************************************************
static void SpiDmaInit(void)
{	
	// enable SPI1 clock
	__SPI1_CLK_ENABLE();
	// enable DMA2 clock
	__DMA1_CLK_ENABLE(); 
	
	// Set the SPI parameters 
  SpiHandle.Instance               = SPI1;
  SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
  SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
  SpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
  SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
  SpiHandle.Init.CRCPolynomial     = 7;
  SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
  SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  SpiHandle.Init.NSS               = SPI_NSS_SOFT;
  SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLED;
  SpiHandle.Init.NSSPMode          = SPI_NSS_PULSE_DISABLED;
  SpiHandle.Init.CRCLength         = SPI_CRC_LENGTH_8BIT;
	SpiHandle.Init.Mode 						 = SPI_MODE_MASTER;
	HAL_SPI_Init(&SpiHandle);
		
	// Configure the DMA handler for SPI1 TX 
	hdma_spi1.Instance 								 = DMA1_Channel3;
	hdma_spi1.Init.Direction					 = DMA_MEMORY_TO_PERIPH;
	hdma_spi1.Init.PeriphInc 					 = DMA_PINC_DISABLE;
	hdma_spi1.Init.MemInc 						 = DMA_MINC_ENABLE;
	hdma_spi1.Init.PeriphDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_spi1.Init.MemDataAlignment 	 = DMA_MDATAALIGN_BYTE;
	hdma_spi1.Init.Mode 							 = DMA_NORMAL;
	hdma_spi1.Init.Priority 				   = DMA_PRIORITY_LOW;
	HAL_DMA_Init(&hdma_spi1); 
  
	// DMA SPI TX
	DMA1_Channel3->CNDTR = BYTES_PER_FRAME + 1;
	DMA1_Channel3->CMAR  = (uint32_t) NULL; 				// src
	DMA1_Channel3->CPAR  = (uint32_t) &(SPI1->DR);  // dest
	
	/* DMA interrupt init */
  HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);	
	
	__HAL_SPI_ENABLE(&SpiHandle);
	
	__HAL_DMA_ENABLE_IT(&hdma_spi1, DMA_IT_TC);	// Enable the transfer complete interrupt
	DMA1->IFCR |= (uint32_t) (DMA_IFCR_CTCIF3); // clear Transfert complete flags
}

// **************************************************************
// 					CeDigitalWrite 
// **************************************************************
static void CeDigitalWrite(uint8_t state)
{
	if (state) 	GPIOB->BSRR |= (uint32_t) GPIO_PIN_1; // set PB_1 HIGH
	else 				GPIOB->BSRR |= (uint32_t) (GPIO_PIN_1 << 16); 
}

// **************************************************************
// 					CsnDigitalWrite 
// **************************************************************
static void CsnDigitalWrite(uint8_t state)
{
	if (state)  GPIOB->BSRR |= (uint32_t) GPIO_PIN_0; // set PB_0 HIGH
	else 			  GPIOB->BSRR |= (uint32_t) (GPIO_PIN_0 << 16); 
}


// **************************************************************
//					Spi1ReturnSend8Bit 
// **************************************************************
static void Spi1ReturnSend8Bit(uint8_t * dataTo, uint8_t * dataFrom, uint8_t length)
{	
  uint8_t indexSpi1;
	CsnDigitalWrite(LOW);
	
	for (indexSpi1=0; indexSpi1 < length; indexSpi1++) 
	{
		*((__IO uint8_t*)(&(SPI1->DR))) = *(dataTo + indexSpi1);
		while( !(SPI1->SR & SPI_FLAG_TXE) ){;} 	// wait until transmit complete
		while( !(SPI1->SR & SPI_FLAG_RXNE) ){;} // wait until receive complete
		while( SPI1->SR & SPI_FLAG_BSY ){;} 		// wait until SPI is not busy anymore
	  *(dataFrom+indexSpi1) = *((__IO uint8_t*)(&(SPI1->DR)));
  }
	CsnDigitalWrite(HIGH);
}

// **************************************************************
//					Spi1Send8Bit 
// **************************************************************
static void Spi1Send8Bit(uint8_t * data, uint8_t length)
{	
  uint8_t indexSpi1;
	CsnDigitalWrite(LOW);
	
	for (indexSpi1=0; indexSpi1<length; indexSpi1++) 
	{
		*((__IO uint8_t*)(&(SPI1->DR))) = *(data+indexSpi1);
		while( !(SPI1->SR & SPI_FLAG_TXE) ); 	// wait until transmit complete
		while( !(SPI1->SR & SPI_FLAG_RXNE) ); // wait until receive complete
		while( SPI1->SR & SPI_FLAG_BSY ); 		// wait until SPI is not busy anymore
	}
	CsnDigitalWrite(HIGH);
}

// **************************************************************
//					Spi1Send8BitThenDma 
// **************************************************************
static void Spi1Send8BitThenDma(uint8_t * data, uint8_t length)
{
  uint8_t indexSpi1;
	CsnDigitalWrite(LOW);
	
	for (indexSpi1=0; indexSpi1<length; indexSpi1++) 
	{
		*((__IO uint8_t*)(&(SPI1->DR))) = *(data+indexSpi1); 	// write data to be transmitted to the SPI data register
		while( !(SPI1->SR & SPI_FLAG_TXE) ); 	// wait until transmit complete
		while( !(SPI1->SR & SPI_FLAG_RXNE) ); // wait until receive complete
		while( SPI1->SR & SPI_FLAG_BSY ); 		// wait until SPI is not busy anymore
	}
}

volatile uint8_t dmaFlag;
/**************************************************************/
// 	 				Spi1DmaSend 
/**************************************************************/
static void Spi1DmaSend(uint8_t * addr) 
{ 	
	static uint8_t transmitMode = W_TX_PAYLOAD;
	Spi1Send8BitThenDma( &transmitMode, 1 );
	
	DMA1_Channel3->CMAR = (uint32_t) (addr) ;
	DMA1_Channel3->CNDTR = BYTES_PER_FRAME;
	
	dmaFlag = 1;
	CsnDigitalWrite(LOW);
	//Enable DMA
	DMA1_Channel3->CCR |= (uint32_t) DMA_CCR_EN;
	//Enable SPI dma
	SPI1->CR2 |= (uint32_t) SPI_CR2_TXDMAEN;	 
	
	while (dmaFlag == 1)
	{
		DataBuffer_Process();
	}
}

// *************************************************************
// 	 				DMA1_Channel2_3_IRQHandler
// *************************************************************
void DMA1_Channel2_3_IRQHandler(void)
{	
	//disable SPI dma
	SPI1->CR2 &= (uint32_t) ~(SPI_CR2_TXDMAEN);
	//Disable DMA
	DMA1_Channel3->CCR &= (uint32_t) ~(DMA_CCR_EN);
	
	// wait SPI isn't busy anymore 
	while( SPI1->SR & SPI_FLAG_BSY ); 

	CsnDigitalWrite(HIGH);	
	
	dmaFlag = 0;
	//clear dma channel 3 transfert complete flag
	DMA1->IFCR |= (uint32_t) DMA_IFCR_CTCIF3;	
}		

static uint8_t packetSent 			= 0;	// number of packet sent
static uint8_t fifo_fill  			= 0; // number of filled Fifo
static uint32_t SecuBreak       = 0;
/**************************************************************/
// 					NRF_SendBuffer
/**************************************************************/
void NRF_SendBuffer(uint8_t * bufferPointer)
{	
	packetSent = 0;
	fifo_fill  = 0;
	SecuBreak = 0;
  
  Check_Reception(); 
	CeDigitalWrite(LOW);
	
	Spi1Send8Bit( TransmitMode, sizeof(TransmitMode) );
	Spi1Send8Bit( &FlushTxFifo, 1 );
	Spi1Send8Bit( ClearIrqFlag, sizeof(ClearIrqFlag) );
	
	while (((packetSent < NUMBER_OF_PACKETS) || (fifo_fill > 0)) && (SecuBreak < 5000))  // while there is data OR a fifo full
	{     
		while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == 1)  // while datas are not sent yet AND there is still data to send
		{  
      SecuBreak++;
      
			if (fifo_fill < 3)     // if all fifo aren't full
			{
				if (packetSent < NUMBER_OF_PACKETS)  // if there is data
				{    
					Spi1DmaSend(bufferPointer);					
					bufferPointer += (BYTES_PER_FRAME);
					packetSent++;
					fifo_fill++;
				}   
			}    
			else 
			{ 				
				if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) != 1) 
				{
					CeDigitalWrite(HIGH);	// ce ==> HIGH, max 10 ms
				} 				
 			}	
				DataBuffer_Process();
      
		}
		Spi1Send8Bit( ClearIrqFlag, sizeof(ClearIrqFlag) );
		fifo_fill--;
		DataBuffer_Process();
	} 	
	CeDigitalWrite(LOW);
	
  Spi1Send8Bit( ReceiveMode, sizeof(ReceiveMode) );
  Spi1Send8Bit( &FlushRxFifo, 1 );
  Spi1Send8Bit( ClearIrqFlag, sizeof(ClearIrqFlag) );
  
  CeDigitalWrite(HIGH);   
}


// **************************************************************
// 					RegisterInit 
// **************************************************************
static void RegisterInit(void)
{
	CeDigitalWrite(LOW);
	
	// sending of the instructions to the NRF
	Spi1Send8Bit(	DisableAutoAck, sizeof(DisableAutoAck));
	Spi1Send8Bit(	RxPayloadSize, 	sizeof(RxPayloadSize)	);
	Spi1Send8Bit(	RxPipe, 				sizeof(RxPipe)				);
	Spi1Send8Bit(	RxTxAdressSize,	sizeof(RxTxAdressSize));
	Spi1Send8Bit(	RxAdressP0,   	sizeof(RxAdressP0)		);
	Spi1Send8Bit(	TxAdress,   		sizeof(TxAdress)			);
	Spi1Send8Bit(	RfChanel, 			sizeof(RfChanel)			);
	Spi1Send8Bit(	RfParameter, 		sizeof(RfParameter)		);
  Spi1Send8Bit(	ClearIrqFlag, 	sizeof(ClearIrqFlag)  );
	Spi1Send8Bit(	&FlushRxFifo, 	1											);
	Spi1Send8Bit(	&FlushTxFifo, 	1											);
	Spi1Send8Bit(	ReceiveMode, 	  sizeof(ReceiveMode)	  );
	CeDigitalWrite(HIGH);
}

/************************************************************************************************************************************/


// **************************************************************
//					Spi1ReturnSend 
// **************************************************************
static void Spi1ReturnSend(uint8_t * dataTo, uint8_t * dataFrom, uint8_t length)
{	
  uint8_t indexSpi1;
	CsnDigitalWrite(LOW);
	
	for (indexSpi1=0; indexSpi1 < length; indexSpi1++) 
	{
		*((__IO uint8_t*)(&(SPI1->DR))) = *(dataTo + indexSpi1) & 0x00FF ;
		
    while( !(SPI1->SR & SPI_FLAG_TXE) )
      {;} 	// wait until transmit complete
		while( !(SPI1->SR & SPI_FLAG_RXNE) )
      {;} // wait until receive complete
		while( SPI1->SR & SPI_FLAG_BSY )
      {;} 		// wait until SPI is not busy anymore
	  
    *(dataFrom+indexSpi1) = *((__IO uint8_t*)(&(SPI1->DR))) & 0x00FF;
  }
	CsnDigitalWrite(HIGH);
}

static uint8_t DataRecep, Reception, ReadStatus = R_REGISTER | STATUS;
static uint8_t ReadPayload[33], Payload[33];
// **************************************************************
// 					Check_Reception
// **************************************************************
void Check_Reception(void)
{ 
  Spi1ReturnSend8Bit(&ReadStatus, &Reception, 1);
  if(Reception & (1<<6))
  {    
    ReadPayload[0] = (R_RX_PAYLOAD);
    Spi1ReturnSend(ReadPayload, Payload,  sizeof(ReadPayload));
    __nop();

    if (Payload[11] == (Payload[10] + 1)) //&& (Payload[13] == Payload[12] + 1))
    {
        DataStateFLAG = 1;
        DataRecep = Payload[4];
        
        if ((DataRecep > 1) && (DataRecep < 5)) 
        DataStateNRF = (DataStateTypeDef) (DataRecep);
        
        else if ((DataRecep >= 100) && (DataRecep <= 200))  
        {
        DataStateNRF = __8ch_3bit__20kHz__C__;
        EtaIndex = DataRecep;
        }
    }
  }   
  Spi1Send8Bit( &FlushRxFifo, 1 );
  Spi1Send8Bit( ClearIrqFlag, sizeof(ClearIrqFlag) );
}


// **************************************************************
// 					NRF_CheckChange
// **************************************************************
uint8_t NRF_CheckChange(void)
{ 
    if(DataStateFLAG)
    {
      DataStateFLAG = 0;
      return 1;
    }
    else 
      return 0;
}

// **************************************************************
// 					NRF_GetDataState
// **************************************************************
DataStateTypeDef NRF_GetDataState(void)
{
    return DataStateNRF;
}

// **************************************************************
// 					NRF_GetEta
// **************************************************************
uint8_t NRF_GetEtaIndex(void)
{
    return EtaIndex;
}

// **************************************************************
// 					NRF_Test
// **************************************************************

void NRF_Test(void)
{	
	static uint8_t spi1TxBuffer[1 + BYTES_PER_FRAME] = 
	{
	  0xA0, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 
	  0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13, 0x14, 
	  0x15, 0x16, 0x17, 0x18, 0x19, 0x20, 0x21, 0x22, 
	  0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x30, 0x31
	};

	CeDigitalWrite(LOW);
	//Spi1Send8Bit(readP0Addr, 	 sizeof(readP0Addr));
	Spi1Send8Bit(ClearIrqFlag, sizeof(ClearIrqFlag));
	Spi1Send8Bit(TransmitMode, sizeof(TransmitMode));
	
	Spi1DmaSend(spi1TxBuffer); 

	CeDigitalWrite(HIGH);	
}














