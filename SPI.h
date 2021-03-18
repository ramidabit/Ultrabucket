/*
 * ECE 153B - Winter 2021
 *
 * Names: Rami Dabit, Kyle Kam
 * Section: Wednesday 7:00-9:50pm
 */

#ifndef __STM32L476G_DISCOVERY_SPI_H
#define __STM32L476G_DISCOVERY_SPI_H

#include "stm32l476xx.h"

#define BufferSize 32

void SPI2_GPIO_Init(void);
void SPI_Init(void);
void SPI_Write(SPI_TypeDef * SPIx, uint8_t *txBuffer, uint8_t * rxBuffer, int size);
void SPI_Read(SPI_TypeDef * SPIx, uint8_t *rxBuffer, int size);
void SPI_Delay(uint32_t us);

#endif 
