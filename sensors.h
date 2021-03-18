/*
 * ECE 153B - Winter 2021
 *
 * Names: Rami Dabit, Kyle Kam
 * Section: Wednesday 7:00-9:50pm
 */

#ifndef SENSORS_H
#define SENSORS_H

#include "stm32l476xx.h"

void Trigger_Setup(void);
void Ch1_Input_Capture_Setup(void);
void Ch2_Input_Capture_Setup(void);
void Ch3_Input_Capture_Setup(void);

#endif 
