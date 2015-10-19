/********************************************************************************
* @file    FBAR.h
* @author  Maxime Clement
* @version V1.0
* @date    01-Mar-2015
* @brief   Header file of the RHD2132 module.
*******************************************************************************/

#ifndef __FBAR_H__
#define __FBAR_H__

#include <stdint.h>
#include <math.h>
#include <CommonDefine.h>

void FBAR_Initialize(uint16_t EtaParam, uint16_t BetaParam);

void FBAR_Reinitialize(uint8_t * bufferTo1, uint8_t * bufferTo2, uint8_t * bufferTo3);

void FBAR_Compress(uint16_t * bufferCompress, uint8_t * bufferSample);

// static uint8_t FBAR_AdaptCutValues(uint8_t channel, int16_t Value);

void FBAR_Dissemble(uint16_t * bufferFrom, uint8_t * bufferTo, DataStateTypeDef DataState);

#endif
