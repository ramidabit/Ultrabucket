/*
 * ECE 153B - Winter 2021
 *
 * Names: Rami Dabit, Kyle Kam
 * Section: Wednesday 7:00-9:50pm
 */

#include "stm32l476xx.h"

void Trigger_Setup() {
	// Enable GPIO port E clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOEEN;
	// Configure PE11 as alternative function (10)
	GPIOE->MODER &= ~GPIO_MODER_MODE11;
	GPIOE->MODER |= GPIO_MODER_MODE11_1;
	// Configure and select alternative function for PE11 (AF1)
	GPIOE->AFR[1] &= ~GPIO_AFRL_AFRL3;
	GPIOE->AFR[1] |= (0x1 << 3*4);
	// Configure PE11 as no pull-up, no pull-down (00)
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

void Ch1_Input_Capture_Setup() {
	// Enable GPIO port A clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	// Configure PA0 as alternative function (10)
	GPIOA->MODER &= ~GPIO_MODER_MODE0;
	GPIOA->MODER |= GPIO_MODER_MODE0_1;
	// Configure and select alternative function for PA0 (AF1)
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFRL0;
	GPIOA->AFR[0] |= 0x1;
	// Configure PA0 as no pull-up, no pull-down (00)
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD0;

	// Enable TIM2 clock
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
	// Set prescaler value to 15
	TIM2->PSC &= ~TIM_PSC_PSC;
	TIM2->PSC |= 0xF;
	// Enable preload for timer 2
	TIM2->CR1 |= TIM_CR1_ARPE;
	// Set auto-reload value to max
	TIM2->ARR |= TIM_ARR_ARR;
	// Configure channel to be used in input capture mode
	// Clear capture and compare mode register
	TIM2->CCMR1 &= ~TIM_CCMR1_CC1S;
	// Set to capture input on timer channel 1
	TIM2->CCMR1 |= TIM_CCMR1_CC1S_0;
	// Enable input capture on both rising and falling edges
	TIM2->CCER |= TIM_CCER_CC1E;
	TIM2->CCER |= TIM_CCER_CC1P;
	TIM2->CCER |= TIM_CCER_CC1NP;
	// Enable interrupts for timer 2
	TIM2->DIER |= TIM_DIER_CC1IE;
	// Enable DMA requests for timer 2
	TIM2->DIER |= TIM_DIER_CC1DE;
	// Enable update interrupt for timer 2
	TIM2->DIER |= TIM_DIER_UIE;
	// Enable update generation
	TIM2->EGR |= 0x1;
	// Clear update interrupt flag
	TIM2->SR &= ~TIM_SR_UIF;
	// Set direction such that timer counts up
	TIM2->CR1 &= ~TIM_CR1_DIR;
	// Enable the counter
	TIM2->CR1 |= 0x1;

	// Enable TIM4_IRQn interrupt in NVIC and set highest priority
	NVIC_EnableIRQ(TIM2_IRQn);
	NVIC_SetPriority(TIM2_IRQn, 2);
}

void Ch2_Input_Capture_Setup() {
	// Please note: Many steps are repeated in case Ch2 is used on its own

	// Enable GPIO port A clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	// Configure PA1 as alternative function (10)
	GPIOA->MODER &= ~GPIO_MODER_MODE1;
	GPIOA->MODER |= GPIO_MODER_MODE1_1;
	// Configure and select alternative function for PA1 (AF1)
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFRL1;
	GPIOA->AFR[0] |= (0x1 << 1*4);
	// Configure PA1 as no pull-up, no pull-down (00)
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD1;

	// Enable TIM2 clock
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
	// Set prescaler value to 15
	TIM2->PSC &= ~TIM_PSC_PSC;
	TIM2->PSC |= 0xF;
	// Enable preload for timer 2
	TIM2->CR1 |= TIM_CR1_ARPE;
	// Set auto-reload value to max
	TIM2->ARR |= TIM_ARR_ARR;
	// Configure channel to be used in input capture mode
	// Clear capture and compare mode register
	TIM2->CCMR1 &= ~TIM_CCMR1_CC2S;
	// Set to capture input on timer channel 2
	TIM2->CCMR1 |= TIM_CCMR1_CC2S_0;
	// Enable input capture on both rising and falling edges
	TIM2->CCER |= TIM_CCER_CC2E;
	TIM2->CCER |= TIM_CCER_CC2P;
	TIM2->CCER |= TIM_CCER_CC2NP;
	// Enable interrupts for timer 2
	TIM2->DIER |= TIM_DIER_CC2IE;
	// Enable DMA requests for timer 2
	TIM2->DIER |= TIM_DIER_CC2DE;
	// Enable update interrupt for timer 2
	TIM2->DIER |= TIM_DIER_UIE;
	// Enable update generation
	TIM2->EGR |= 0x1;
	// Clear update interrupt flag
	TIM2->SR &= ~TIM_SR_UIF;
	// Set direction such that timer counts up
	TIM2->CR1 &= ~TIM_CR1_DIR;
	// Enable the counter
	TIM2->CR1 |= 0x1;

	// Enable TIM4_IRQn interrupt in NVIC and set highest priority
	NVIC_EnableIRQ(TIM2_IRQn);
	NVIC_SetPriority(TIM2_IRQn, 2);
}

void Ch3_Input_Capture_Setup() {
	// Please note: Many steps are repeated in case Ch3 is used on its own

	// Enable GPIO port A clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	// Configure PA2 as alternative function (10)
	GPIOA->MODER &= ~GPIO_MODER_MODE2;
	GPIOA->MODER |= GPIO_MODER_MODE2_1;
	// Configure and select alternative function for PA2 (AF1)
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFRL2;
	GPIOA->AFR[0] |= (0x1 << 2*4);
	// Configure PA2 as no pull-up, no pull-down (00)
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD2;

	// Enable TIM2 clock
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
	// Set prescaler value to 15
	TIM2->PSC &= ~TIM_PSC_PSC;
	TIM2->PSC |= 0xF;
	// Enable preload for timer 2
	TIM2->CR1 |= TIM_CR1_ARPE;
	// Set auto-reload value to max
	TIM2->ARR |= TIM_ARR_ARR;
	// Configure channel to be used in input capture mode
	// Clear capture and compare mode register
	TIM2->CCMR2 &= ~TIM_CCMR2_CC3S;
	// Set to capture input on timer channel 3
	TIM2->CCMR2 |= TIM_CCMR2_CC3S_0;
	// Enable input capture on both rising and falling edges
	TIM2->CCER |= TIM_CCER_CC3E;
	TIM2->CCER |= TIM_CCER_CC3P;
	TIM2->CCER |= TIM_CCER_CC3NP;
	// Enable interrupts for timer 2
	TIM2->DIER |= TIM_DIER_CC3IE;
	// Enable DMA requests for timer 2
	TIM2->DIER |= TIM_DIER_CC3DE;
	// Enable update interrupt for timer 2
	TIM2->DIER |= TIM_DIER_UIE;
	// Enable update generation
	TIM2->EGR |= 0x1;
	// Clear update interrupt flag
	TIM2->SR &= ~TIM_SR_UIF;
	// Set direction such that timer counts up
	TIM2->CR1 &= ~TIM_CR1_DIR;
	// Enable the counter
	TIM2->CR1 |= 0x1;

	// Enable TIM4_IRQn interrupt in NVIC and set highest priority
	NVIC_EnableIRQ(TIM2_IRQn);
	NVIC_SetPriority(TIM2_IRQn, 2);
}
