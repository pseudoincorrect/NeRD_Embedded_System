
#ifndef __COMMONDEFINE_H__
#define __COMMONDEFINE_H__

//#define DEBUG 

#define COMPRESS // enable compression

#define LOW		0
#define HIGH	1

#define CHANNEL_SIZE 	0x08 // must be a multiple of BYTES_PER_FRAME (NRF.h)

#define BYTES_PER_FRAME	   32 // ammount of byte in one packet transmission
#define NUMBER_OF_PACKETS	 20 // number of packet send at once (size of the buffer)
#define SAMPLE_BUFFER_SIZE 	(BYTES_PER_FRAME * NUMBER_OF_PACKETS) // defined in NRF.h

#define SIZE_BUFFER_RHD			50	
#define SIZE_BUFFER_NRF			4

#define NBIT 				 	 3								 // resolution of the compression
#define POW_2_NBIT  	(1 << NBIT) 			// 2^NBIT
#define CUT_VAL_SIZE 	(POW_2_NBIT - 1) // number of cut value
#define ETA					 	 600						// adaptation parameter

#define DEBUG_HIGH 	(GPIOA->BSRR |= GPIO_PIN_15)
#define DEBUG_LOW		(GPIOA->BSRR |= ((uint32_t) GPIO_PIN_15 << 16))


#endif
