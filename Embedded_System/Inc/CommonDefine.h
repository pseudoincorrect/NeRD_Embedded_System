
#ifndef __COMMONDEFINE_H__
#define __COMMONDEFINE_H__

//#define DEBUG 

#define COMPRESS // enable compression

#define LOW		0
#define HIGH	1

#define CHANNEL_SIZE 	0x08 // must be a multiple of BYTES_PER_FRAME (NRF.h)

#define BYTES_PER_FRAME	   32 // ammount of byte in one packet transmission
#define NUMBER_OF_PACKETS	 30 // number of packet (size of the buffer)
#define SAMPLE_BUFFER_SIZE 	(BYTES_PER_FRAME * NUMBER_OF_PACKETS) // defined in NRF.h

#define SIZE_BUFFER_RHD			10	
#define SIZE_BUFFER_NRF			10

#define NBIT 				 	 2								 // resolution of the compression
#define POW_2_NBIT  	(1 << NBIT) 			// 2^NBIT
#define CUT_VAL_SIZE 	(POW_2_NBIT - 1) // number of cut value
#define ETA					 	 0.1						// adaptation parameter

#endif
