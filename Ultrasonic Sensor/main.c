/*
 * ECE 153B - Summer 2020
 *
 * Name: Rami Dabit
 * Section: Wednesday 7:00-9:50pm
 * Lab: 3B
 */
 
#include <stdio.h> 
 
#include "stm32l476xx.h"
#include "lcd.h"

uint32_t volatile currentValue = 0;
uint32_t volatile lastValue = 0;
uint32_t volatile overflowCount = 0;
uint32_t volatile timeInterval = 0;

void Trigger_Setup() {
	// Enable GPIO port E clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOEEN;
	// Configure PE11 as alternative function (10)
	GPIOE->MODER &= ~GPIO_MODER_MODE11;
	GPIOE->MODER |= GPIO_MODER_MODE11_1;
	// Configure and select alternative function for PE11 (AF1)
	GPIOE->AFR[1] &= ~GPIO_AFRL_AFRL3;
	GPIOE->AFR[1] |= (0x1 << 3*4);
	// Configure PE8 as no pull-up, no pull-down (00)
	GPIOE->PUPDR &= ~GPIO_PUPDR_PUPD11;
	// Configure PE11 output type as push-pull (0)
	GPIOE->OTYPER &= ~GPIO_OTYPER_OT11;
	// Configure PE11 output speed as very high (11)
	GPIOE->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR11;

	// Enable TIM1 clock
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

	// Set prescaler value to 15
	TIM1->PSC &= ~TIM_PSC_PSC;
	TIM1->PSC |= 0xF;

	// Enable preload for TIM1 CH 2
	TIM1->CR1 |= TIM_CR1_ARPE;
	// Set auto-reload value to 20
	TIM1->ARR &= ~TIM_ARR_ARR;
	TIM1->ARR |= 0x14;

	// Set capture and compare value to 10
	TIM1->CCR2 &= ~TIM_CCR2_CCR2;
	TIM1->CCR2 |= 0xA;

	// Configure channel to be used in output compare mode
	// Clear the output compare mode bits for channel 2
	TIM1->CCMR1 &= ~TIM_CCMR1_OC2M;
	// Set output control mode bits to PWM mode 1
	TIM1->CCMR1 |= TIM_CCMR1_OC2M_1;
	TIM1->CCMR1 |= TIM_CCMR1_OC2M_2;
	// Enable output compare preload
	TIM1->CCMR1 |= TIM_CCMR1_OC2PE;

	// Enable channel 2 output
	TIM1->CCER |= TIM_CCER_CC2E;

	// Enable main output and off-state selection
	TIM1->BDTR |= TIM_BDTR_MOE | TIM_BDTR_OSSR;

	// Enable update generation
	TIM1->EGR |= 0x1;

	// Enable update interrupt for timer 4
	TIM1->DIER |= TIM_DIER_UIE;
	// Clear update interrupt flag
	TIM1->SR &= ~TIM_SR_UIF;

	// Set direction such that timer counts up
	TIM1->CR1 &= ~TIM_CR1_DIR;
	// Enable the counter
	TIM1->CR1 |= 0x1;
}

void Input_Capture_Setup() {
	// Enable GPIO port B clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	// Configure PB6 as alternative function (10)
	GPIOB->MODER &= ~GPIO_MODER_MODE6;
	GPIOB->MODER |= GPIO_MODER_MODE6_1;
	// Configure and select alternative function for PB6 (AF2)
	GPIOB->AFR[0] &= ~GPIO_AFRL_AFRL6;
	GPIOB->AFR[0] |= (0x2 << 6*4);
	// Configure PB6 as no pull-up, no pull-down (00)
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD6;

	// Enable TIM4 clock
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM4EN;

	// Set prescaler value to 15
	TIM4->PSC &= ~TIM_PSC_PSC;
	TIM4->PSC |= 0xF;

	// Enable preload for TIM4 CH 1
	TIM4->CR1 |= TIM_CR1_ARPE;
	// Set auto-reload value to max
	TIM4->ARR |= TIM_ARR_ARR;

	// Configure channel to be used in input capture mode
	// Clear capture and compare mode register
	TIM4->CCMR1 &= ~TIM_CCMR1_CC1S;
	// Set to capture input on timer channel 1
	TIM4->CCMR1 |= TIM_CCMR1_CC1S_0;

	// Enable input capture on both rising and falling edges
	TIM4->CCER |= TIM_CCER_CC1E;
	TIM4->CCER |= TIM_CCER_CC1P;
	TIM4->CCER |= TIM_CCER_CC1NP;

	// Enable interrupts for timer 4
	TIM4->DIER |= TIM_DIER_CC1IE;
	// Enable DMA requests for timer 4
	TIM4->DIER |= TIM_DIER_CC1DE;
	// Enable update interrupt for timer 4
	TIM4->DIER |= TIM_DIER_UIE;

	// Enable update generation
	TIM4->EGR |= 0x1;

	// Clear update interrupt flag
	TIM4->SR &= ~TIM_SR_UIF;

	// Set direction such that timer counts up
	TIM4->CR1 &= ~TIM_CR1_DIR;

	// Enable the counter
	TIM4->CR1 |= 0x1;

	// Enable TIM4_IRQn interrupt in NVIC and set priority to 2
	NVIC_EnableIRQ(TIM4_IRQn);
	NVIC_SetPriority(TIM4_IRQn, 2);
}

void TIM4_IRQHandler(void) {
	// currentValue = CCR_New
	// lastValue = CCR_Last
	// overflowCount = OC
	// timeInterval = CCR_New - CCR_Last + (1+ARR)*OC
	//  Note that we set ARR to the max: 1+(2^16-1) = 65536

	if(TIM4->SR & TIM_SR_UIF) { // UIF Interrupt
		TIM4->SR &= ~TIM_SR_UIF;
		overflowCount++;

	} else if(TIM4->SR & TIM_SR_CC1IF) { // Channel 1
		TIM4->SR &= ~TIM_SR_CC1IF;

		if(GPIOB->IDR & GPIO_IDR_ID6) { // Echo pin high (rising edge)
			lastValue = TIM4->CCR1 & TIM_CCR1_CCR1;
			
		} else { // Echo pin low (falling edge)
			currentValue = TIM4->CCR1 & TIM_CCR1_CCR1;

			// Compute pulse width and reset overflow count
			timeInterval = currentValue - lastValue + 65536*overflowCount;
			overflowCount = 0;
		}
	}
}

int main(void) {	
	// Enable High Speed Internal Clock (HSI = 16 MHz)
	RCC->CR |= RCC_CR_HSION;
	while ((RCC->CR & RCC_CR_HSIRDY) == 0); // Wait until HSI is ready
	
	// Select HSI as system clock source 
	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_HSI;
	while ((RCC->CFGR & RCC_CFGR_SWS) == 0); // Wait until HSI is system clock source
  
	// Input Capture Setup
	Input_Capture_Setup();
	
	// Trigger Setup
	Trigger_Setup();

	// Setup LCD
	LCD_Initialization();
	LCD_Clear();
	
	char message[6];
	while(1) {
		// The HC-SR04 sensor can measure distances between 2cm and 400cm

		if(timeInterval <= 58*400 && timeInterval >= 58*2) {
			// Convert sensor measurements to a distance in cm
			sprintf(message, "%6d", timeInterval / 58);
		} else {
			// No object in range
			sprintf(message, "%s", "------");
		}
		
		LCD_DisplayString((uint8_t *) message);
	}
}
