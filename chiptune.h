// This code comes from https://github.com/deater/vmw-meter/tree/master/stm32L476/chiptune_cs43l22

#include <stdint.h>
#include "stm32l476xx.h"
#include "string.h"
#include "stdlib.h"
#include "i2c.h"
#include "cs43l22.h"
#include "sysclock.h"
#include <math.h>

#define FREQ	44100
#define CHANS	2
#define BITS	16

#define AUDIO_BUFSIZ (FREQ*CHANS*(BITS/8)/50)
#define NUM_SAMPLES (AUDIO_BUFSIZ/CHANS/(BITS/8))
#define COUNTDOWN_RESET (FREQ/50)

static uint16_t audio_buf[AUDIO_BUFSIZ*2];

static void DMA2_Channel6_IRQHandler(void);
void DMA_Init(void);
void SAI_Init(void);
int i2s_transmit(uint16_t *datal, uint16_t *datar, uint16_t size);
void NextBuffer(int which_half);
