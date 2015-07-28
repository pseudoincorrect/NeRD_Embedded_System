#include "stm32f0xx_hal.h"
#include "NRF.h"
#include "SampleSend.h"
#include "board_interface.h"
#include "CommonDefine.h"

// *************************************************************************
// *************************************************************************
// 						Private functions	
// *************************************************************************
// *************************************************************************
static void ChangeDataState(void);
static void InitBufferTest(void);
// *************************************************************************
// *************************************************************************
// 						static variables	
// *************************************************************************
// *************************************************************************
static DataStateTypeDef DataState = FIRST_STATE;
extern DataStateTypeDef DataStateNRF;
static uint8_t EtaIndex;
extern volatile uint16_t Fbar_Eta;

static uint8_t BufferTest[NUMBER_OF_PACKETS][BYTES_PER_FRAME];

// **************************************************************
// 	 				MAIN 
// **************************************************************
int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  Board_Init();
 
	/* Initialize the NRF moduke */	
  NRF_Init();
  
  InitBufferTest();
  
	while (1)
  {
    NRF_SendBuffer((uint8_t *) BufferTest);
  } 
}

// **************************************************************
// 	 				ChangeDataState 
// **************************************************************
static void ChangeDataState(void)
{
  SampleSend_Enable(LOW);
  
  if (DataState != NRF_GetDataState())
  {
    DataState = NRF_GetDataState();
    EtaIndex = NRF_GetEtaIndex();
    SampleSend_SetState(DataState);
    DataBuffer_ChangeState(DataState, (uint8_t) EtaIndex);
    //DataBuffer_ChangeState(DataState, (uint8_t) 0);
    SampleSend_SetState(DataState);
  }
  
  if ((DataState == __8ch_3bit__20kHz__C__ ) && (EtaIndex != NRF_GetEtaIndex()))
  {
    EtaIndex = NRF_GetEtaIndex();
    DataBuffer_ChangeState(__8ch_3bit__20kHz__C__, (uint8_t) EtaIndex);
    //DataBuffer_ChangeState(__8ch_3bit__20kHz__C__, (uint8_t) 0);
    SampleSend_SetState(__8ch_3bit__20kHz__C__);
  }   
  SampleSend_Enable(HIGH);
}

// **************************************************************
// 	 				InitBufferTest 
// **************************************************************
static void InitBufferTest(void)
{
  uint8_t i,j;
  
  for(i=0; i < NUMBER_OF_PACKETS; i++)
  {
    for(j=0; j < BYTES_PER_FRAME; j++)
    { 
      if(i & 0x01) 
      {
        if (j == 0)
          BufferTest[i][j] = 0x0E;
        if (j == 1)
          BufferTest[i][j] = 0x0F;
        else
          BufferTest[i][j] = j + 100;
      }
      else
      {
        if (j == 0)
          BufferTest[i][j] = 0xE0;
        if (j == 1)
          BufferTest[i][j] = 0xF0;
        else
          BufferTest[i][j] = j + 100;
      }
    }
  }
}











