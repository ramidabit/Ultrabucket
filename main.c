/*
 * ECE 153B - Winter 2021
 *
 * Names: Rami Dabit, Kyle Kam
 * Section: Wednesday 7:00-9:50pm
 */

#include "stm32l476xx.h"
#include "UART.h"
#include "LCD.h"
#include "SPI.h"
#include "L3GD20.h"
#include "SysClock.h"
#include "SysTimer.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "string.h"
#include "stdlib.h"
#include "I2C.h"
#include "cs43l22.h"
#include "chiptune.h"
#include "sensors.h"

// Threshold for toggling functionality using gyroscope
int THRESHOLD = 300;

uint32_t volatile currentValue_1 = 0;
uint32_t volatile currentValue_2 = 0;
uint32_t volatile currentValue_3 = 0;
uint32_t volatile lastValue_1 = 0;
uint32_t volatile lastValue_2 = 0;
uint32_t volatile lastValue_3 = 0;
uint32_t volatile overflowCount_1 = 0;
uint32_t volatile overflowCount_2 = 0;
uint32_t volatile overflowCount_3 = 0;
uint32_t volatile timeInterval_1 = 0;
uint32_t volatile timeInterval_2 = 0;
uint32_t volatile timeInterval_3 = 0;

typedef struct {
	float x; 
	float y; 
	float z; 
} L3GD20_Data_t;

L3GD20_Data_t currentSpeed;

void TIM2_IRQHandler(void) {
	// currentValue = CCR_New
	// lastValue = CCR_Last
	// overflowCount = OC
	// timeInterval = CCR_New - CCR_Last + (1+ARR)*OC
	//  Note that we set ARR to the max: 1+(2^16-1) = 65536

	if(TIM2->SR & TIM_SR_UIF) { // UIF Interrupt
		TIM2->SR &= ~TIM_SR_UIF;
		overflowCount_1++;
	} else {
		if(TIM2->SR & TIM_SR_CC1IF) { // Channel 1
			TIM2->SR &= ~TIM_SR_CC1IF;

			if(GPIOA->IDR & GPIO_IDR_ID0) { // Echo pin high (rising edge)
				lastValue_1 = TIM2->CCR1 & TIM_CCR1_CCR1;
				
			} else { // Echo pin low (falling edge)
				currentValue_1 = TIM2->CCR1 & TIM_CCR1_CCR1;

				// Compute pulse width and reset overflow count
				timeInterval_1 = currentValue_1 - lastValue_1 + 65536*overflowCount_1;
				overflowCount_1 = 0;
			}
		}
		if(TIM2->SR & TIM_SR_CC2IF) { // Channel 2
			TIM2->SR &= ~TIM_SR_CC2IF;

			if(GPIOA->IDR & GPIO_IDR_ID1) { // Echo pin high (rising edge)
				lastValue_2 = TIM2->CCR2 & TIM_CCR2_CCR2;
				
			} else { // Echo pin low (falling edge)
				currentValue_2 = TIM2->CCR2 & TIM_CCR2_CCR2;

				// Compute pulse width and reset overflow count
				timeInterval_2 = currentValue_2 - lastValue_2 + 65536*overflowCount_2;
				overflowCount_2 = 0;
			}
		}
		if(TIM2->SR & TIM_SR_CC3IF) { // Channel 3
			TIM2->SR &= ~TIM_SR_CC3IF;

			if(GPIOA->IDR & GPIO_IDR_ID2) { // Echo pin high (rising edge)
				lastValue_3 = TIM2->CCR3 & TIM_CCR3_CCR3;
				
			} else { // Echo pin low (falling edge)
				currentValue_3 = TIM2->CCR3 & TIM_CCR3_CCR3;

				// Compute pulse width and reset overflow count
				timeInterval_3= currentValue_3 - lastValue_3 + 65536*overflowCount_3;
				overflowCount_3 = 0;
			}
		}
	}
}

void PWM_Init() {
    // Enable GPIO port E clock
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOEEN;
    // Configure PE8 as alternative function (10)
    GPIOE->MODER &= ~GPIO_MODER_MODE8;
    GPIOE->MODER |= GPIO_MODER_MODE8_1;
    // Configure and select alternative function for PE8 (AF1)
    GPIOE->AFR[1] &= ~GPIO_AFRL_AFRL0;
    GPIOE->AFR[1] |= 0x1;
    // Configure PE8 output speed as very high (11)
    GPIOE->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR8;
    // Configure PE8 as no pull-up, no pull-down (00)
    GPIOE->PUPDR &= ~GPIO_PUPDR_PUPD8;

    // Enable TIM1 clock
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    // Configure PWM Output for TIM1 CH 1N
    // Set direction such that timer counts down
    TIM1->CR1 |= TIM_CR1_DIR;
    // Set prescaler value
    TIM1->PSC &= ~TIM_PSC_PSC;
    // Set auto-reload value
    TIM1->ARR &= ~TIM_ARR_ARR;
    TIM1->ARR |= 0x13;
    // Configure channel to be used in output compare mode
    // Clear the output compare mode bits
    TIM1->CCMR1 &= ~TIM_CCMR1_OC1M;
    // Set output compare mode bits to PWM mode 1
    TIM1->CCMR1 |= TIM_CCMR1_OC1M_1;
    TIM1->CCMR1 |= TIM_CCMR1_OC1M_2;
    // Enable output preload
    TIM1->CCMR1 |= TIM_CCMR1_OC1PE;
    // Set the output polarity for compare 1 to active high
    TIM1->CCER &= ~TIM_CCER_CC1NP;
    // Enable channel 1N output
    TIM1->CCER |= TIM_CCER_CC1NE;
    // Enable main output
    TIM1->BDTR |= TIM_BDTR_MOE;
    // Set capture/compare value for duty cycle of PWM output
    TIM1->CCR1 &= ~TIM_CCR1_CCR1;
    TIM1->CCR1 |= 0x7;
    // Enable the counter
    TIM1->CR1 |= 0x1;
}

void setStationary(int xStat, int yStat, int zStat) {
	// Calibrate X, Y, and Z by adjusting toward the opposite direction
	currentSpeed.x += -1*xStat;
	currentSpeed.y += -1*yStat;
	currentSpeed.x += -1*zStat;
};

void getSpeed(uint8_t highAddr, uint8_t lowAddr, uint8_t direction) {
	int16_t speed = 0;

	// Check current status of the gyroscope
	uint8_t temp = 0;
	GYRO_IO_Read(&temp, L3GD20_STATUS_REG_ADDR, sizeof(temp));
	// Make sure there is a new speed to read
	if(temp & direction) {
		temp = 0;			// Read in upper 8 bits
		GYRO_IO_Read(&temp, highAddr, sizeof(temp));
		speed |= temp << 8;
		temp = 0;			// Read in lower 8 bits
		GYRO_IO_Read(&temp, lowAddr, sizeof(temp));
		speed |= temp;
	}

	// If speed is negative, convert out of 2's complement
	if(speed & 0x80) speed = -1*(~speed + 0x1);
	// Convert to degrees per second
	speed = 0.07*speed;

	if(direction & 0x1) {
		currentSpeed.x = speed;
	} else if(direction & 0x2) {
		currentSpeed.y = speed;
	} else if(direction & 0x4) {
		currentSpeed.z = speed;
	}
}

void updateSpeed() {
	// Results are stored in currentSpeed global variable
	getSpeed(L3GD20_OUT_X_H_ADDR, L3GD20_OUT_X_L_ADDR, 0x1);
	getSpeed(L3GD20_OUT_Y_H_ADDR, L3GD20_OUT_Y_L_ADDR, 0x2);
	getSpeed(L3GD20_OUT_Z_H_ADDR, L3GD20_OUT_Z_L_ADDR, 0x4);
};

void enterLowPowerState() {
	// Disable the GPIO ports and timers for the ultrasonic sensors
	RCC->AHB2ENR &= ~RCC_AHB2ENR_GPIOEEN;
	RCC->AHB2ENR &= ~RCC_AHB2ENR_GPIOAEN;
	RCC->APB1ENR1 &= ~RCC_APB1ENR1_TIM2EN;
	RCC->APB2ENR &= ~RCC_APB2ENR_TIM1EN;
}

void exitLowPowerState() {
	// Enable the GPIO ports and timers for the ultrasonic sensors
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOEEN;
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
}

int main(void) {
	// Enable High Speed Internal Clock (HSI = 16 MHz)
	RCC->CR |= RCC_CR_HSION;
	// Select HSI as system clock source 
	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_HSI;
	// Wait until HSI is system clock source and ready
	while ((RCC->CFGR & RCC_CFGR_SWS) == 0);
	while ((RCC->CR & RCC_CR_HSIRDY) == 0);

	SysTick_Init();
	System_Clock_Init();

	LCD_Initialization();
	LCD_Clear();

	UART2_Init();
	UART2_GPIO_Init();
	USART_Init(USART2);

	PWM_Init();

	// Trigger Setup (The same trigger is used for all sensors)
	Trigger_Setup();
	// Input Capture Setup
	Ch1_Input_Capture_Setup();
	Ch2_Input_Capture_Setup();
	Ch3_Input_Capture_Setup();

	// Initialize Gyroscope
	GYRO_Init();
	// Initalize currentSpeed
	updateSpeed();

	uint8_t data_receive[6];
	uint8_t data_send[6];
	int slave_addr;

	data_receive[0]=0;

	I2C_GPIO_Init();
	I2C_Initialization();
	SAI_Init();
	cs43l22_init();

	slave_addr=0x94;
	data_send[0]=CS43L22_REG_ID;
	I2C_SendData(I2C1,slave_addr,data_send,1);
	I2C_ReceiveData(I2C1,slave_addr,data_receive,1);
	
	// Initialize DAC for audio
	cs43l22_play();

	bool active = 0;
	bool led_on = 0;
	bool sensor1 = 0;
	bool sensor2 = 0;
	bool sensor3 = 0;
	char message[6];
	while(1) {
		// Note: Our X value is -1 when the board is stationary, so we must calibrate
		//  or adjust the velocity values based on the gyroscope's default readings
		setStationary(-1, 0, 0);

		// Use gyroscope reading to toggle functionality
		if(currentSpeed.x > THRESHOLD || currentSpeed.x < -1*THRESHOLD ||
			 currentSpeed.y > THRESHOLD || currentSpeed.y < -1*THRESHOLD) {
			active = !active;

			// Wait 3 seconds for user to be able to put on hat
			//  without it toggling its status a second time
			delay(3000);
		}

		if(active) {
			sprintf(message, "%s", "  ON  ");
			if(!led_on) {
				TIM1->CCR1 &= ~TIM_CCR1_CCR1;
				TIM1->CCR1 |= 0xD;
				led_on = 1;
			}
			exitLowPowerState();
			DMA_Init();

			// The HC-SR04 sensor can measure distances between 2cm and 400cm
			sensor1 = (timeInterval_1 < 58*400 && timeInterval_1 >= 58*2) ? 1 : 0;
			sensor2 = (timeInterval_2 < 58*400 && timeInterval_2 >= 58*2) ? 1 : 0;
			sensor3 = (timeInterval_3 < 58*400 && timeInterval_3 >= 58*2) ? 1 : 0;

			// Convert sensor measurements to a distance in cm, or "--" if range is invalid
			/*
			if(sensor1 && sensor2 && sensor3) {
				sprintf(message, "%2d%2d%2d", timeInterval_1/58, timeInterval_2/58, timeInterval_3/58);
			} else if(sensor1 && sensor2 && !sensor3) {
				sprintf(message, "%2d%2d%s", timeInterval_1/58, timeInterval_2/58, "--");
			} else if(sensor1 && !sensor2 && sensor3) {
				sprintf(message, "%2d%s%2d", timeInterval_1/58, "--", timeInterval_3/58);
			} else if(sensor1 && !sensor2 && !sensor3) {
				sprintf(message, "%2d%s", timeInterval_1/58, "----");
			} else if(!sensor1 && sensor2 && sensor3) {
				sprintf(message, "%s%2d%2d", "--", timeInterval_2/58, timeInterval_3/58);
			} else if(!sensor1 && sensor2 && !sensor3) {
				sprintf(message, "%s%2d%s", "--", timeInterval_2/58, "--");
			} else if(!sensor1 && !sensor2 && sensor3) {
				sprintf(message, "%s%2d", "----", timeInterval_3/58);
			} else {
				sprintf(message, "%s", "------");
			}
			*/

			if(sensor1 || sensor2 || sensor3) {
				if(timeInterval_1 < 58*50 ||
				   timeInterval_2 < 58*50 ||
				   timeInterval_3 < 58*50) {
					// If any direction within 50cm, play high frequency beeps
					NextBuffer(0);
					NextBuffer(1);
				} else if(timeInterval_1 < 58*100 ||
						  timeInterval_2 < 58*100 ||
						  timeInterval_3 < 58*100) {
					// If any direction within 100cm, play medium beeps
					NextBuffer(2);
					NextBuffer(3);
				} else if(timeInterval_1 < 58*150 ||
						  timeInterval_2 < 58*150 ||
						  timeInterval_3 < 58*150) {
					// If any direction within 150cm, play slow beeps
					NextBuffer(4);
					NextBuffer(5);
				} else {
					// If no object in range, then silence
					NextBuffer(6);
					NextBuffer(7);
				}
			}
		} else {
			sprintf(message, "%s", " OFF  ");
			if(led_on) {
				TIM1->CCR1 &= ~TIM_CCR1_CCR1;
				TIM1->CCR1 |= 0x7;
				led_on = 0;
			}
			NextBuffer(6);
			NextBuffer(7);
		}
		
		LCD_DisplayString((uint8_t *) message);

		// Connection to Termite available for testing and debugging
		printf("Current Angular Velocity\n   X: %.01f\n   Y: %.01f\n   Z: %.01f\n\n",
				currentSpeed.x, currentSpeed.y, currentSpeed.z);
		
		updateSpeed();
		delay(500); // Small delay between receiving measurements
	}
}
