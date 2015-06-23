#ifndef RHD2000_H
#define RHD2000_H

#include <CommonDefine.h>

#define MASK_CONVERT 		0x0000 
#define MASK_CALIBRATE 	0x5500
#define MASK_CLEAR 			0x6A00
#define MASK_WRITE 			0x8000
#define MASK_READ				0xC000

/************************************************************/
// Register number
#define RHD2000_R0 	0x0000 // Register 0
#define RHD2000_R1 	0x0100 // Register 1	
#define RHD2000_R2 	0x0200 // ...
#define RHD2000_R3 	0x0300
#define RHD2000_R4 	0x0400
#define RHD2000_R5 	0x0500
#define RHD2000_R6 	0x0600
#define RHD2000_R7 	0x0700
#define RHD2000_R8 	0x0800
#define RHD2000_R9 	0x0900
#define RHD2000_R10 0x0A00
#define RHD2000_R11 0x0B00
#define RHD2000_R12 0x0C00
#define RHD2000_R13 0x0D00
#define RHD2000_R14 0x0E00
#define RHD2000_R15 0x0F00
#define RHD2000_R16 0x1000
#define RHD2000_R17 0x1100 // Register 17

/************************************************************/
/* Register configurations: Datas that will contain all registers 
configuration for RHD2216 and RHD2132 these registers can be change, 
you can check the details on RHD2000 datasheet page 19 to 22 */

// R0: ADC Configuration and Amplifier Fast Settle
#define DATA_R0 	0x00DE 	// 1101 1110

// R1: Supply sensor and ADC Buffer Bias Current
#define DATA_R1 	0x0020 	// 0010 0000

// R2: MUX Bias Current
#define DATA_R2 	0x0028 	// 0010 1000

// R3: MUX Load, Temperature Sensor, and Auxiliary Digital Output
#define DATA_R3 	0x0000 	// 0000 0000

// R4: ADC Output Format and DSP Offset Removal
#define DATA_R4 	0x009F 	// 1001 1111

// R5: Impedance Check Control
#define DATA_R5 	0x0000 	// 0000 0000

// R6: Impedance Check DAC
#define DATA_R6 	0x0000 	// 0000 0000

// R7: Impedance Check Amplifier Select
#define DATA_R7 	0x0000 	// 0000 0000

// R8-11: On-Chip Amplifier Bandwith Select upper frequency
#define DATA_R8 	0x0021 	// 0010 1010
#define DATA_R9 	0x0000 	// 0000 1010
#define DATA_R10  0x0025 	// 0000 0101
#define DATA_R11  0x0000 	// 0000 1101

// R12-13: On-Chip Amplifier Bandwith Select lower frequency
#define DATA_R12  0x0019 	// 0001 0000
#define DATA_R13  0x0000 	// 0111 1100

#ifdef SWITCH_CHANNEL
  // R14-17: Individual Amplifier Power select the input used in RHD2000
  // Amplifiers 0 to 7 			 	 7  6  5  4  3  2  1  0
  #define DATA_R14  0x002A 	// 0  0  1  0  1  0  1  0
  // Amplifiers 8 to 15        15 14 13 12 11 10 9  8
  #define DATA_R15  0x0000 	// 0  0  0  0  0  0  0  0
  // Amplifiers 16 to 23		   23 22 21 20 19 18 17 16
  #define DATA_R16  0x0000 	// 0  0  0  0  0  0  0  0
  // Amplifiers 24 to 31       31 30 29 28 27 26 25 24
  #define DATA_R17  0x0074 	// 0  1  1  1  0  1  0  0
#else
  // R14-17: Individual Amplifier Power select the input used in RHD2000
  // Amplifiers 0 to 7 			 	 7  6  5  4  3  2  1  0
  #define DATA_R14  0x001D 	// 0  0  0  1  1  1  0  1
  // Amplifiers 8 to 15        15 14 13 12 11 10 9  8
  #define DATA_R15  0x0000 	// 0  0  0  0  0  0  0  0
  // Amplifiers 16 to 23		   23 22 21 20 19 18 17 16
  #define DATA_R16  0x0000 	// 0  0  0  0  0  0  0  0
  // Amplifiers 24 to 31       31 30 29 28 27 26 25 24
  #define DATA_R17  0x00A8 	// 1  0  1  0  1  0  0  0
#endif
/************************************************************/

#endif









