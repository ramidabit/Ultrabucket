/*
 * ECE 153B - Winter 2021
 *
 * Name: Rami Dabit
 * Section: Wednesday 7:00-9:50pm
 * Lab: 4B
 */

#include "SPI.h"
#include "SysTimer.h"

// Note: When the data frame size is 8 bit, "SPIx->DR = byte_data;" works incorrectly. 
// It mistakenly send two bytes out because SPIx->DR has 16 bits. To solve the program,
// we should use "*((volatile uint8_t*)&SPIx->DR) = byte_data";

// LSM303C eCompass (a 3D accelerometer and 3D magnetometer module) SPI Interface: 
//   MAG_DRDY = PC2   MEMS_SCK  = PD1 (SPI2_SCK)   XL_CS  = PE0             
//   MAG_CS   = PC0   MEMS_MOSI = PD4 (SPI2_MOSI)  XL_INT = PE1       
//   MAG_INT  = PC1 
//
// L3GD20 Gyro (three-axis digital output) SPI Interface: 
//   MEMS_SCK  = PD1 (SPI2_SCK)    GYRO_CS   = PD7 (GPIO)
//   MEMS_MOSI = PD4 (SPI2_MOSI)   GYRO_INT1 = PD2
//   MEMS_MISO = PD3 (SPI2_MISO)   GYRO_INT2 = PB8

extern uint8_t Rx1_Counter;
extern uint8_t Rx2_Counter;

void SPI2_GPIO_Init(void) {
	// Enable GPIO port D clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIODEN;
	// Configure PD1 as alternative function (10)
	GPIOD->MODER &= ~GPIO_MODER_MODE1;
	GPIOD->MODER |= GPIO_MODER_MODE1_1;
	// Configure PD3 as alternative function (10)
	GPIOD->MODER &= ~GPIO_MODER_MODE3;
	GPIOD->MODER |= GPIO_MODER_MODE3_1;
	// Configure PD4 as alternative function (10)
	GPIOD->MODER &= ~GPIO_MODER_MODE4;
	GPIOD->MODER |= GPIO_MODER_MODE4_1;
	// Configure and select alternative function for PD1 (AF5)
	GPIOD->AFR[0] &= ~GPIO_AFRL_AFRL1;
	GPIOD->AFR[0] |= (0x5 << 1*4);
	// Configure and select alternative function for PD3 (AF5)
	GPIOD->AFR[0] &= ~GPIO_AFRL_AFRL3;
	GPIOD->AFR[0] |= (0x5 << 3*4);
	// Configure and select alternative function for PD4 (AF5)
	GPIOD->AFR[0] &= ~GPIO_AFRL_AFRL4;
	GPIOD->AFR[0] |= (0x5 << 4*4);
	// Configure PD1 as no pull-up/pull-down (00)
	GPIOD->PUPDR &= ~GPIO_PUPDR_PUPD1;
	// Configure PD3 as no pull-up/pull-down (00)
	GPIOD->PUPDR &= ~GPIO_PUPDR_PUPD3;
	// Configure PD4 as no pull-up/pull-down (00)
	GPIOD->PUPDR &= ~GPIO_PUPDR_PUPD4;
	// Configure PD1 output type as push-pull (0)
	GPIOD->OTYPER &= ~GPIO_OTYPER_OT1;
	// Configure PD3 output type as push-pull (0)
	GPIOD->OTYPER &= ~GPIO_OTYPER_OT3;
	// Configure PD4 output type as push-pull (0)
	GPIOD->OTYPER &= ~GPIO_OTYPER_OT4;
	// Configure PD1 output speed as very high (11)
	GPIOD->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR1;
	// Configure PD3 output speed as very high (11)
	GPIOD->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR3;
	// Configure PD4 output speed as very high (11)
	GPIOD->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR4;
}

void SPI_Init(void){
	// Enable SPI2 clock in peripheral clock register
	RCC->APB1ENR1 |= RCC_APB1ENR1_SPI2EN;

	// Reset SPI2 in peripheral reset register
	RCC->APB1RSTR1 |= RCC_APB1RSTR1_SPI2RST;
	// Clear SPI2RST to exit reset state
	RCC->APB1RSTR1 &= ~RCC_APB1RSTR1_SPI2RST;

	// Disable SPI
	SPI2->CR1 &= ~SPI_CR1_SPE;

	// Configure serial channel for full duplex communication
	SPI2->CR1 &= ~SPI_CR1_RXONLY;

	// Configure communication in 2-line unidirectional data mode
	SPI2->CR1 &= ~SPI_CR1_BIDIMODE;

	// Disable output in bidirectional mode
	SPI2->CR1 &= ~SPI_CR1_BIDIOE;

	// Set frame format for receiving MSB first
	SPI2->CR1 &= ~SPI_CR1_LSBFIRST;
	// Set data length to 8 bits
	SPI2->CR2 |= SPI_CR2_DS;
	SPI2->CR2 &= ~SPI_CR2_DS_3;
	// Set frame format to SPI Motorola mode
	SPI2->CR2 &= ~SPI_CR2_FRF;

	// Set clock polarity to 0
	SPI2->CR1 &= ~SPI_CR1_CPOL;
	// Set clock phase so first clock transition is first data capture edge
	SPI2->CR1 &= ~SPI_CR1_CPHA;

	// Set baud rate prescaler to 16
	SPI2->CR1 &= ~SPI_CR1_BR;
	SPI2->CR1 |= SPI_CR1_BR_0;
	SPI2->CR1 |= SPI_CR1_BR_1;

	// Disable CRC calculation
	SPI2->CR1 &= ~SPI_CR1_CRCEN;

	// Set board to master mode
	SPI2->CR1 |= SPI_CR1_MSTR;
	// Enable software peripheral management, NSS pulse management
	SPI2->CR1 |= SPI_CR1_SSM;
	SPI2->CR2 |= SPI_CR2_NSSP;

	// Set internal peripheral select bit
	SPI2->CR1 |= SPI_CR1_SSI;

	// Set FIFO reception threshold to 1/4 or 8 bit
	SPI2->CR2 |= SPI_CR2_FRXTH;

	// Re-enable SPI
	SPI2->CR1 |= SPI_CR1_SPE;
}
 
void SPI_Write(SPI_TypeDef * SPIx, uint8_t *txBuffer, uint8_t * rxBuffer, int size) {
	volatile uint32_t tmpreg; 
	int i = 0;
	for (i = 0; i < size; i++) {
		while( (SPIx->SR & SPI_SR_TXE ) != SPI_SR_TXE );  // Wait for TXE (Transmit buffer empty)
		*((volatile uint8_t*)&SPIx->DR) = txBuffer[i];
		while((SPIx->SR & SPI_SR_RXNE ) != SPI_SR_RXNE); // Wait for RXNE (Receive buffer not empty)
		rxBuffer[i] = *((__IO uint8_t*)&SPIx->DR);
	}
	while( (SPIx->SR & SPI_SR_BSY) == SPI_SR_BSY ); // Wait for BSY flag cleared
}

void SPI_Read(SPI_TypeDef * SPIx, uint8_t *rxBuffer, int size) {
	int i = 0;
	for (i = 0; i < size; i++) {
		while( (SPIx->SR & SPI_SR_TXE ) != SPI_SR_TXE ); // Wait for TXE (Transmit buffer empty)
		*((volatile uint8_t*)&SPIx->DR) = rxBuffer[i];	
		// The clock is controlled by master. Thus the master has to send a byte
		// data to the slave to start the clock. 
		while((SPIx->SR & SPI_SR_RXNE ) != SPI_SR_RXNE); 
		rxBuffer[i] = *((__IO uint8_t*)&SPIx->DR);
	}
	while( (SPIx->SR & SPI_SR_BSY) == SPI_SR_BSY ); // Wait for BSY flag cleared
}

void SPI_Delay(uint32_t us) {
	uint32_t i, j;
	for (i = 0; i < us; i++) {
		for (j = 0; j < 18; j++) // This is an experimental value.
			(void)i;
	}
}
