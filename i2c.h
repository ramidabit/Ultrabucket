/*
 * ECE 153B - Winter 2021
 *
 * Names: Rami Dabit, Kyle Kam
 * Section: Wednesday 7:00-9:50pm
 */

#ifndef __STM32L476G_DISCOVERY_I2C_H
#define __STM32L476G_DISCOVERY_I2C_H

#include <stdint.h>
#include <cstddef>
#include "stm32l476xx.h"

#define READ_FROM_SLAVE 1
#define WRITE_TO_SLAVE  0

int8_t I2C_Start(I2C_TypeDef * I2Cx, uint32_t DevAddress, uint8_t Size, uint8_t Direction);
void I2C_Stop(I2C_TypeDef * I2Cx);
void I2C_WaitLineIdle(I2C_TypeDef * I2Cx);
int8_t I2C_SendData(I2C_TypeDef * I2Cx, uint8_t DeviceAddress, uint8_t *pData, uint8_t Size);
int8_t I2C_ReceiveData(I2C_TypeDef * I2Cx, uint8_t DeviceAddress, uint8_t *pData, uint8_t Size);
void I2C_GPIO_Init(void);
void I2C_Initialization(void);
void CODEC_Initialization(void);

#endif
