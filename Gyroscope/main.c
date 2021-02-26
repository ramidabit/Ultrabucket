/*
 * ECE 153B - Winter 2021
 *
 * Name: Rami Dabit
 * Section: Wednesday 7:00-9:50pm
 * Lab: 4B
 */

#include "stm32l476xx.h"

#include "L3GD20.h"
#include "SPI.h"
#include "SysClock.h"
#include "SysTimer.h"
#include "UART.h"

#include <stdio.h>

typedef struct {
	float x; 
	float y; 
	float z; 
} L3GD20_Data_t;

L3GD20_Data_t currentSpeed;

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

int main(void){
	System_Clock_Init();   // System Clock = 80 MHz
	SysTick_Init();
	
	// Initialize Gyroscope
	GYRO_Init();  

	// Initialize USART2
	UART2_Init();
	UART2_GPIO_Init();
	USART_Init(USART2);

	// Initalize currentSpeed
	updateSpeed();
	
	while(1) {
		// Note: For some reason, my X value is -1 when my board is not moving, so I chose
		//  to calibrate/adjust my values based on the gyroscope's default readings
		setStationary(-1, 0, 0);

		printf("Current Angular Velocity\n   X: %.01f\n   Y: %.01f\n   Z: %.01f\n\n",
				currentSpeed.x, currentSpeed.y, currentSpeed.z);
		updateSpeed();
		delay(500); // Small delay between receiving measurements
	}
}
