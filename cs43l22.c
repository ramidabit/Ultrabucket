// This code comes from https://github.com/deater/vmw-meter/tree/master/stm32L476/chiptune_cs43l22

#include <stdint.h>
#include "stm32l476xx.h"
#include "i2c.h"
#include "cs43l22.h"

void cs43l22_init(void) {
	volatile int delay;
	int slave_addr=0x94;
	unsigned char sound_data[2];
	int reset_pin=3;
	int mclk_pin=2,sclk_pin=5,sd_pin=6,fs_pin=4;

	/* PB6/PB7 is SCL/SDA. Those are initialized in i2c.c */
	/* PE3 is Reset */
	/* PE2/PE5/PE6/PE4 is SAI1 MCK/SCK/SD/FS */

	/* Enable RESET pin              */
	/* Enable GPIOE                  */
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOEEN;
	/* Set to output */
	GPIOE->MODER &= ~(3UL<<(reset_pin*2));
	GPIOE->MODER |= 1UL<<(reset_pin*2);
	/* Hold |reset low until power supplies are stable */
	GPIOE->ODR&=~(1<<reset_pin);
	for(delay=0;delay<4000;delay++);
	/* Bring |reset high */
	GPIOE->ODR|= (1<<reset_pin);
	for(delay=0;delay<4000;delay++);

	/* CS43L22 INIT                  */
	/* be sure power crl register (0x2) is 0x1 */
	sound_data[0]=CS43L22_REG_POWER_CTL1;		/* 0x02 */
	sound_data[1]=0x01;
	I2C_SendData(I2C1,slave_addr,sound_data,2);
	/* Write 0x99 to register 0x00 */
	sound_data[0]=0x00;
	sound_data[1]=0x99;
	I2C_SendData(I2C1,slave_addr,sound_data,2);
	/* Write 0x80 to register 0x47 */
	sound_data[0]=0x47;
	sound_data[1]=0x80;
	I2C_SendData(I2C1,slave_addr,sound_data,2);
	/* Write 1 to bit 7 in register 0x32 */
	sound_data[0]=0x32;
	sound_data[1]=0xbb;
	I2C_SendData(I2C1,slave_addr,sound_data,2);
	/* Write 0 to bit 7 in register 0x32 */
	sound_data[0]=0x32;
	sound_data[1]=0x3b;
	I2C_SendData(I2C1,slave_addr,sound_data,2);
	/* Write 0 to regsiter 0x00 */
	sound_data[0]=0x00;
	sound_data[1]=0x00;
	I2C_SendData(I2C1,slave_addr,sound_data,2);

	/* Configure GPIOE for PE2(MCLK), PE5(SCLK) and PE6(SDIN) on I2S */
	/* PE4=(FS) */
	/* Alternate function 13 */

	/* Set mode of pins as alternate function */
	/* 00 = input, 01 = output, 10 = alternate, 11 = analog (def) */
	GPIOE->MODER &= ~(3UL<<(mclk_pin*2));
	GPIOE->MODER |= 2UL<<(mclk_pin*2);
	GPIOE->MODER &= ~(3UL<<(sclk_pin*2));
	GPIOE->MODER |= 2UL<<(sclk_pin*2);
	GPIOE->MODER &= ~(3UL<<(sd_pin*2));
	GPIOE->MODER |= 2UL<<(sd_pin*2);
	GPIOE->MODER &= ~(3UL<<(fs_pin*2));
	GPIOE->MODER |= 2UL<<(fs_pin*2);

	/* Set alternate function 13 (SAI1) */
	GPIOE->AFR[0] &= ~0x00000f00;
	GPIOE->AFR[0] |=  0x00000d00;
	GPIOE->AFR[0] &= ~0x000f0000;
	GPIOE->AFR[0] |=  0x000d0000;
	GPIOE->AFR[0] &= ~0x00f00000;
	GPIOE->AFR[0] |=  0x00d00000;
	GPIOE->AFR[0] &= ~0x0f000000;
	GPIOE->AFR[0] |=  0x0d000000;

	/* Set as no pull-up/pull-down */
	/* 00 = no pull-up, no pull-down, 01 = pull-up, 10 = pull-down */
	GPIOE->PUPDR &=~(3UL<<(mclk_pin*2));
	GPIOE->PUPDR &=~(3UL<<(sclk_pin*2));
	GPIOE->PUPDR &=~(3UL<<(sd_pin*2));
	GPIOE->PUPDR &=~(3UL<<(fs_pin*2));

	/* Set output as push/pull  */
	GPIOE->OTYPER &=~(1<<mclk_pin);
	GPIOE->OTYPER &=~(1<<sclk_pin);
	GPIOE->OTYPER &=~(1<<sd_pin);
	GPIOE->OTYPER &=~(1<<fs_pin);

	/* Set speed to (very high) */
	GPIOE->OSPEEDR |= (3<<(mclk_pin*2));
	GPIOE->OSPEEDR |= (3<<(sclk_pin*2));
	GPIOE->OSPEEDR |= (3<<(sd_pin*2));
	GPIOE->OSPEEDR |= (3<<(fs_pin*2));

	/* Delay as clock settles */
	for(delay=0;delay<4000;delay++);

	/* Clock configuration: Auto detection */
	sound_data[0]=CS43L22_REG_CLOCKING_CTL;		/* 0x05 */
	sound_data[1]=0x80;

	I2C_SendData(I2C1,slave_addr,sound_data,2);

	/* Set the Slave Mode and the audio Standard */
	/* CODEC_STANDARD 04 = i2s, 32-bit */
	/* 07 = slave, not inv, dsp disabled, i2s, 16-bit */
	sound_data[0]=CS43L22_REG_INTERFACE_CTL1;	/* 0x06 */
	sound_data[1]=0x04;
	I2C_SendData(I2C1,slave_addr,sound_data,2);

	/* Set the Master volume */
	cs43l22_set_volume(60);

	/* Set the Headphone Gain */
	sound_data[0]=CS43L22_REG_PLAYBACK_CTL1;	/* 0x0d */
	sound_data[1]=(6<<5);
	I2C_SendData(I2C1,slave_addr,sound_data,2);

	/* Setup ramp, which lets the codec power down gracefully */
	/* Otherwise you might be stuck with high-pitch noise */

	/* Disable the analog soft ramp */
	sound_data[0]=CS43L22_REG_ANALOG_ZC_SR_SETT;	/* 0x0A */
	sound_data[1]=0x00;
	I2C_SendData(I2C1,slave_addr,sound_data,2);
	/* Disable the digital soft ramp */
	sound_data[0]=CS43L22_REG_MISC_CTL;		/* 0x0E */
	sound_data[1]=0x04;
	I2C_SendData(I2C1,slave_addr,sound_data,2);
	/* Disable the limiter attack level */
	sound_data[0]=CS43L22_REG_LIMIT_CTL1;		/* 0x27 */
	sound_data[1]=0x00;
	I2C_SendData(I2C1,slave_addr,sound_data,2);
	/* Adjust Bass and Treble levels */
	sound_data[0]=CS43L22_REG_TONE_CTL;		/* 0x1f */
	sound_data[1]=0x0f;
	I2C_SendData(I2C1,slave_addr,sound_data,2);
	/* Adjust PCM volume level */
	sound_data[0]=CS43L22_REG_PCMA_VOL;		/* 0x1a */
	sound_data[1]=0xf;
	I2C_SendData(I2C1,slave_addr,sound_data,2);
	sound_data[0]=CS43L22_REG_PCMB_VOL;		/* 0x1b */
	sound_data[1]=0xf;
	I2C_SendData(I2C1,slave_addr,sound_data,2);

}

void cs43l22_set_volume(int volume) {
	int slave_addr=0x94;
	unsigned char sound_data[2];

	uint8_t convertedvol = (((volume) > 100)? 100:((uint8_t)(((volume) * 255) / 100)));

	if (volume > 0xe6) {
		sound_data[0]=CS43L22_REG_MASTER_A_VOL;		/* 0x20 */
		sound_data[1]=convertedvol-0xe7;
		I2C_SendData(I2C1,slave_addr,sound_data,2);

		sound_data[0]=CS43L22_REG_MASTER_B_VOL;		/* 0x21 */
		sound_data[1]=convertedvol-0xe7;
		I2C_SendData(I2C1,slave_addr,sound_data,2);
	} else {
		sound_data[0]=CS43L22_REG_MASTER_A_VOL;
		sound_data[1]=convertedvol+0x19;
		I2C_SendData(I2C1,slave_addr,sound_data,2);

		sound_data[0]=CS43L22_REG_MASTER_B_VOL;
		sound_data[1]=convertedvol+0x19;
		I2C_SendData(I2C1,slave_addr,sound_data,2);
	}

	/* Make sure headphones are unmuted */
	sound_data[0]=CS43L22_REG_HEADPHONE_A_VOL;		/* 0x22 */
	sound_data[1]=0x00;
	I2C_SendData(I2C1,slave_addr,sound_data,2);

	sound_data[0]=CS43L22_REG_HEADPHONE_B_VOL;		/* 0x23 */
	sound_data[1]=0x00;
	I2C_SendData(I2C1,slave_addr,sound_data,2);
}

void cs43l22_play(void) {
	unsigned char sound_data[2];
	int slave_addr=0x94;

	/* Enable Output device, register 4 */
	/* Headphones always on, speaker always off: write 10101111 */
	sound_data[0]=CS43L22_REG_POWER_CTL2;		/* 0x04 */
	sound_data[1]=0xaf;
	I2C_SendData(I2C1,slave_addr,sound_data,2);

	/* Power on the Codec */
	/* Write 0x9e to register 0x02 */
	sound_data[0]=CS43L22_REG_POWER_CTL1;		/* 0x02 */
	sound_data[1]=0x9e;
	I2C_SendData(I2C1,slave_addr,sound_data,2);

}

void cs43l22_beep(void) {
	unsigned char sound_data[2];
	int slave_addr=0x94;

	/* Unmute, set volume to middle */
	sound_data[0]=CS43L22_REG_PCMA_VOL;		/* 0x1a */
	sound_data[1]=0x00;
	I2C_SendData(I2C1,slave_addr,sound_data,2);
	/* Unmute, set volume to middle */
	sound_data[0]=CS43L22_REG_PCMB_VOL;		/* 0x1b */
	sound_data[1]=0x00;
	I2C_SendData(I2C1,slave_addr,sound_data,2);

	/* Beep volume and off time */
	/* Regular volume, off 1.2s */
	sound_data[0]=CS43L22_REG_BEEP_VOL_OFF_TIME;		/* 0x1d */
	sound_data[1]=(0x0<<5)|(0);
	I2C_SendData(I2C1,slave_addr,sound_data,2);

	sound_data[0]=CS43L22_REG_BEEP_FREQ_ON_TIME;		/* 0x1C */
	/* 1kHz, on 1.2s */
	sound_data[1]=(0x7<<4)|(0x3);
	I2C_SendData(I2C1,slave_addr,sound_data,2);

	sound_data[0]=CS43L22_REG_BEEP_TONE_CFG;		/* 0x1E */
	sound_data[1]=(0x3<<6)|(0x0);
	I2C_SendData(I2C1,slave_addr,sound_data,2);
}

void cs43l22_disable(void) {}
