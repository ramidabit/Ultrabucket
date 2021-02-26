/*
 * ECE 153B - Winter 2021
 *
 * Name: Rami Dabit
 * Section: Wednesday 7:00-9:50pm
 * Lab: 4B
 */

#include "UART.h"

void UART1_Init(void) {
	// Enable USART1 clock in peripheral clock register
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	// Select SYSCLK in clock configuration register
	RCC->CFGR &= ~RCC_CFGR_MCOSEL;
	RCC->CFGR |= RCC_CFGR_MCOSEL_0;
}

void UART2_Init(void) {
	// Enable USART2 clock in peripheral clock register
	RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
	// Select SYSCLK in clock configuration register
	RCC->CFGR &= ~RCC_CFGR_MCOSEL;
	RCC->CFGR |= RCC_CFGR_MCOSEL_0;
}

void UART1_GPIO_Init(void) {
	// Enable GPIO port B clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	// Configure PB6 as alternative function (10)
	GPIOB->MODER &= ~GPIO_MODER_MODE6;
	GPIOB->MODER |= GPIO_MODER_MODE6_1;
	// Configure PB7 as alternative function (10)
	GPIOB->MODER &= ~GPIO_MODER_MODE7;
	GPIOB->MODER |= GPIO_MODER_MODE7_1;
	// Configure and select alternative function for PB6 (AF7)
	GPIOB->AFR[0] &= ~GPIO_AFRL_AFRL6;
	GPIOB->AFR[0] |= (0x7 << 6*4);
	// Configure and select alternative function for PB7 (AF7)
	GPIOB->AFR[0] &= ~GPIO_AFRL_AFRL7;
	GPIOB->AFR[0] |= (0x7 << 7*4);
	// Configure PB6 as pull-up (01)
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD6;
	GPIOB->PUPDR |= GPIO_PUPDR_PUPD6_0;
	// Configure PB7 as pull-up (01)
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD7;
	GPIOB->PUPDR |= GPIO_PUPDR_PUPD7_0;
	// Configure PB6 output type as push-pull (0)
	GPIOB->OTYPER &= ~GPIO_OTYPER_OT6;
	// Configure PB7 output type as push-pull (0)
	GPIOB->OTYPER &= ~GPIO_OTYPER_OT7;
	// Configure PB6 output speed as very high (11)
	GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR6;
	// Configure PB7 output speed as very high (11)
	GPIOB->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR7;
}

void UART2_GPIO_Init(void) {
	// Enable GPIO port D clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIODEN;
	// Configure PD5 as alternative function (10)
	GPIOD->MODER &= ~GPIO_MODER_MODE5;
	GPIOD->MODER |= GPIO_MODER_MODE5_1;
	// Configure PD6 as alternative function (10)
	GPIOD->MODER &= ~GPIO_MODER_MODE6;
	GPIOD->MODER |= GPIO_MODER_MODE6_1;
	// Configure and select alternative function for PD5 (AF7)
	GPIOD->AFR[0] &= ~GPIO_AFRL_AFRL5;
	GPIOD->AFR[0] |= (0x7 << 5*4);
	// Configure and select alternative function for PD6 (AF7)
	GPIOD->AFR[0] &= ~GPIO_AFRL_AFRL6;
	GPIOD->AFR[0] |= (0x7 << 6*4);
	// Configure PD5 as pull-up (01)
	GPIOD->PUPDR &= ~GPIO_PUPDR_PUPD5;
	GPIOD->PUPDR |= GPIO_PUPDR_PUPD5_0;
	// Configure PD6 as pull-up (01)
	GPIOD->PUPDR &= ~GPIO_PUPDR_PUPD6;
	GPIOD->PUPDR |= GPIO_PUPDR_PUPD6_0;
	// Configure PD5 output type as push-pull (0)
	GPIOD->OTYPER &= ~GPIO_OTYPER_OT5;
	// Configure PD6 output type as push-pull (0)
	GPIOD->OTYPER &= ~GPIO_OTYPER_OT6;
	// Configure PD5 output speed as very high (11)
	GPIOD->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR5;
	// Configure PD6 output speed as very high (11)
	GPIOD->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR6;
}

void USART_Init(USART_TypeDef* USARTx) {
	// Disable USART
	USARTx->CR1 &= ~USART_CR1_UE;

	// Set word length to 8 bits
	USARTx->CR1 &= ~USART_CR1_M;
	// Set oversampling mode to oversample by 16
	USARTx->CR1 &= ~USART_CR1_OVER8;
	// Set number of stop bits to 1
	USARTx->CR2 &= ~USART_CR2_STOP;

	// Set baud rate to 9600
	USARTx->BRR = (80000000/9600);

	// Enable USART transmitter
	USARTx->CR1 |= USART_CR1_TE;
	// Enable USART receiver
	USARTx->CR1 |= USART_CR1_RE;

	// Re-enable USART
	USARTx->CR1 |= USART_CR1_UE;
}

uint8_t USART_Read (USART_TypeDef * USARTx) {
	// SR_RXNE (Read data register not empty) bit is set by hardware
	while (!(USARTx->ISR & USART_ISR_RXNE));  // Wait until RXNE (RX not empty) bit is set
	// USART resets the RXNE flag automatically after reading DR
	return ((uint8_t)(USARTx->RDR & 0xFF));
	// Reading USART_DR automatically clears the RXNE flag 
}

void USART_Write(USART_TypeDef * USARTx, uint8_t *buffer, uint32_t nBytes) {
	int i;
	// TXE is cleared by a write to the USART_DR register.
	// TXE is set by hardware when the content of the TDR 
	// register has been transferred into the shift register.
	for (i = 0; i < nBytes; i++) {
		while (!(USARTx->ISR & USART_ISR_TXE));   	// wait until TXE (TX empty) bit is set
		// Writing USART_DR automatically clears the TXE flag 	
		USARTx->TDR = buffer[i] & 0xFF;
		USART_Delay(300);
	}
	while (!(USARTx->ISR & USART_ISR_TC));   		  // wait until TC bit is set
	USARTx->ISR &= ~USART_ISR_TC;
}   

void USART_Delay(uint32_t us) {
	uint32_t time = 100*us/7;    
	while(--time);   
}
