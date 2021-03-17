// This code comes from https://github.com/deater/vmw-meter/tree/master/stm32L476/chiptune_cs43l22

#include "chiptune.h"
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

volatile uint32_t TimeDelay;
volatile uint32_t overflows = 0;
int counter = 0;

/* mono (1 channel), 16-bit (2 bytes), play at 50Hz */
#define AUDIO_BUFSIZ (FREQ*CHANS*(BITS/8)/50)
#define NUM_SAMPLES (AUDIO_BUFSIZ/CHANS/(BITS/8))
#define COUNTDOWN_RESET (FREQ/50)

void NextBuffer(int which_half) {
	float PI = 3.141516;
	int line_decode_result=0;
	
	float coef = (float)2.0 * PI * 100 / (float)FREQ;  

	/* Decode next frame */
	if (which_half == 0) {
        for(int i=0; i<AUDIO_BUFSIZ; i++) {
                audio_buf[i] = (uint16_t)(32766*sinf((900*coef*i)+32768));
            }
    }
    else if (which_half == 1){
        for(int i=0; i<AUDIO_BUFSIZ; i++) {
                audio_buf[AUDIO_BUFSIZ + i] = (uint16_t)(32766*sinf(900*coef*i)+32768);
            }
    }
    else if (which_half == 2){
        for(int i=0; i<AUDIO_BUFSIZ; i++) {
                audio_buf[i] = (uint16_t)(32766*sinf((440*coef*i)+32768));
            }
    }
    else if (which_half == 3){
        for(int i=0; i<AUDIO_BUFSIZ; i++) {
                audio_buf[AUDIO_BUFSIZ + i] = (uint16_t)(32766*sinf(440*coef*i)+32768);
            }
    }
    else if (which_half == 4){
        for(int i=0; i<AUDIO_BUFSIZ; i++) {
                audio_buf[i] = (uint16_t)(32766*sinf((110*coef*i)+32768));
            }
    }
    else if (which_half == 5){
        for(int i=0; i<AUDIO_BUFSIZ; i++) {
                audio_buf[AUDIO_BUFSIZ + i] = (uint16_t)(32766*sinf(110*coef*i)+32768);
            }
    }
    else if (which_half == 6){
        for(int i=0; i<AUDIO_BUFSIZ; i++) {
                audio_buf[i] = (uint16_t)(32768);
            }
    }
    else if (which_half == 7){
        for(int i=0; i<AUDIO_BUFSIZ; i++) {
                audio_buf[AUDIO_BUFSIZ + i] = (uint16_t)(32768);
            }
    }
}

static void DMA_IRQHandler(void) {
	/* This is called at both half-full and full DMA */
	/* We double buffer */

	/* At half full, we should load next buffer at start */
	/* At full, we should load next buffer to end */

	if ((DMA2->ISR&DMA_ISR_TCIF6)==DMA_ISR_TCIF6) {
		NextBuffer(1);
		/* This should happen at roughly 50Hz */
	}

	if ((DMA2->ISR&DMA_ISR_HTIF6)==DMA_ISR_HTIF6) {
		NextBuffer(0);
		/* This should happen at roughly 50Hz */
	}

	/* ACK interrupt */
	/* Set to 1 to clear */
	DMA2->IFCR |= DMA_IFCR_CGIF6;
}

// Set up DMA for SAI1_A (Controller 2 Trigger 1 Channel 6)
void DMA_Init(void) {
	/* Enable DMA2 clock */
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
	/* Disable DMA2 Channel 6 */
	DMA2_Channel6->CCR &= ~DMA_CCR_EN;
	/* Peripheral data size = 32 bits */
	DMA2_Channel6->CCR &= ~DMA_CCR_PSIZE;
	DMA2_Channel6->CCR |= DMA_CCR_PSIZE_1;
	/* Memory data size = 16 bits */
	DMA2_Channel6->CCR &= ~DMA_CCR_MSIZE;
	DMA2_Channel6->CCR |= DMA_CCR_MSIZE_1;
	/* Disable peripheral increment mode */
	DMA2_Channel6->CCR &=~DMA_CCR_PINC;
	/* Enable memory increment mode */
	DMA2_Channel6->CCR |= DMA_CCR_MINC;
	/* Transfer Direction: to perihpheral */
	DMA2_Channel6->CCR |= DMA_CCR_DIR;
	/* Circular Buffer */
	DMA2_Channel6->CCR |= DMA_CCR_CIRC;
	/* Amount of data to transfer */
	DMA2_Channel6->CNDTR = AUDIO_BUFSIZ;
	/* Peripheral Address */
	DMA2_Channel6->CPAR = (uint32_t)&(SAI1_Block_A->DR);
	/* Memory Address */
	DMA2_Channel6->CMAR = (uint32_t)&(audio_buf);
	/* Set up Channel Select */
	DMA2_CSELR->CSELR &= ~DMA_CSELR_C6S;
	/* SAI1_A */
	DMA2_CSELR->CSELR |= 1<<20;
	/* Enable Half-done interrupt */
	DMA2_Channel6->CCR |= DMA_CCR_HTIE;
	/* Enable Transfer complete interrupt */
	DMA2_Channel6->CCR |= DMA_CCR_TCIE;
	/* Enable DMA2 Channel 6 */
	DMA2_Channel6->CCR |= DMA_CCR_EN;

	return;
}

// Set up SAI for 16-bit stereo i2s
void SAI_Init(void) {
	int krg2=0;

	/* Enable the clock for SAI1 */
	RCC->APB2ENR |= RCC_APB2ENR_SAI1EN;

	/* CR1 register */
		/* Disable SAI while configuring */
		SAI1_Block_A->CR1 &= ~SAI_xCR1_SAIEN;
		/* Set "Free" Protocol */
		SAI1_Block_A->CR1 &= ~SAI_xCR1_PRTCFG;
		/* Set Master Transmitter */
		SAI1_Block_A->CR1 &= ~SAI_xCR1_MODE;
		/* Set first bit MSB */
		SAI1_Block_A->CR1 &= ~SAI_xCR1_LSBFIRST;
		/* Transmit, so set clock strobing to falling edge */
		SAI1_Block_A->CR1|= SAI_xCR1_CKSTR;
		/* Set datasize to 16 */
		SAI1_Block_A->CR1&= ~SAI_xCR1_DS;
		SAI1_Block_A->CR1|= 4<<5; // unsure
		/* Set MCKDIV */
		SAI1_Block_A->CR1 &= ~(SAI_xCR1_MCKDIV);

	/* CR2 register */
		/* Set FIFO Threshold to 1/4 full */
		SAI1_Block_A->CR2 &= ~SAI_xCR2_FTH;
		SAI1_Block_A->CR2 |= 1;


	/* FRCR register */
		/* Frame Length = 32, manual says subtract 1 */
		SAI1_Block_A->FRCR |= (32-1);  /* two 16-bit values */
		/* Frame offset before first bit */
		SAI1_Block_A->FRCR |= SAI_xFRCR_FSOFF;
		/* SAI_FS_CHANNEL_IDENTIFICATION */
		SAI1_Block_A->FRCR |= SAI_xFRCR_FSDEF;
		/* Frame Polarity active low*/
		SAI1_Block_A->FRCR &= ~SAI_xFRCR_FSPOL;
		/* ActiveFrameLength = 16, manual says subtract 1*/
		SAI1_Block_A->FRCR |= (16-1)<<8;

	/* SLOTR register */
		/* Slot first bit offset = 0 */
		SAI1_Block_A->SLOTR &= ~SAI_xSLOTR_FBOFF; 
		/* Slot size = 16bits */
		SAI1_Block_A->SLOTR &= ~SAI_xSLOTR_SLOTSZ;
		SAI1_Block_A->SLOTR |= 1<<6;
		/* Slot active = 0x3 */
		SAI1_Block_A->SLOTR |= (0x3<<16);
		/* Slot number = 2 (stereo) */
		SAI1_Block_A->SLOTR &= ~SAI_xSLOTR_NBSLOT;
		SAI1_Block_A->SLOTR |= (2-1)<<8;

    /* DMA */
		SAI1_Block_A->CR1 |= SAI_xCR1_DMAEN;
		
	/* Enable */
	SAI1_Block_A->CR1 |= SAI_xCR1_SAIEN;

	krg2=SAI1_Block_A->CR1;
}

/* Blocking transmit */
int i2s_transmit(uint16_t *datal, uint16_t *datar, uint16_t size) { //whats this do

	uint32_t count=size;
	uint32_t temp;
	uint16_t *left_data=datal;
	uint16_t *right_data=datar;
	int left=1;

	if ((datar==NULL) || (datal==NULL) || (size==0)) return -1;

	/* If not enabled, fill FIFO and enable */
	if ((SAI1_Block_A->CR1 & SAI_xCR1_SAIEN)==0) {
		while (((SAI1_Block_A->SR & SAI_xSR_FLVL) != (SAI_xSR_FLVL_0 & ~SAI_xSR_FLVL_1 & SAI_xSR_FLVL_2))//FIX SAI_FIFOSTATUS_FULL
			&& (count > 0U)) {

			if (left) {
				temp = (uint32_t)(*left_data);
				left_data++;
				left=0;
			}
			else {
				temp = ((uint32_t)(*right_data));
				right_data++;
				left=1;
			}
			SAI1_Block_A->DR = temp;

			count--;
		}
		SAI1_Block_A->CR1 |= SAI_xCR1_SAIEN;
	}

	while (count > 0U) {
		/* Write data if the FIFO is not full */
		if ((SAI1_Block_A->SR & SAI_xSR_FLVL) != (SAI_xSR_FLVL_0 & ~SAI_xSR_FLVL_1 & SAI_xSR_FLVL_2)) { // FIX FIFOSTATUS AGAIN
			if (left) {
				temp = (uint32_t)(*left_data);
				left_data++;
				left=0;
			}
			else {
				temp = (uint32_t)(*right_data);
				right_data++;
				left=1;
			}
			SAI1_Block_A->DR = temp;
			count--;
			GPIOE->ODR &= ~(1<<8);
		}
		else {
			GPIOE->ODR |= (1<<8);
		}
	}

	/* Check for the Timeout */

	return 0;
}

/* Note: no need to touch the code beyond this point */

#if 0
/* Set 80MHz PLL based on 16MHz HSI clock */
void System_Clock_Init(void) {

        /* Enable the HSI clock */
        RCC->CR |= RCC_CR_HSION;

	/* Wait until HSI is ready */
	while ( (RCC->CR & RCC_CR_HSIRDY) == 0 );

	/* Set the FLASH latency to 4 (BLARGH!) */
	/* Need to set *some* wait states if you are running >16MHz */
	/* See 3.3.3 (p100) in manual */
        FLASH->ACR |= FLASH_ACR_LATENCY_4WS;

	/* Configure PLL */
	RCC->CR &= ~RCC_CR_PLLON;
	while((RCC->CR & RCC_CR_PLLRDY) == RCC_CR_PLLRDY);

	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLSRC;
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSI;

/*
	The system Clock is configured as follows :
            System Clock source            = PLL (HSI)
            AHB Prescaler                  = 1
            APB1 Prescaler                 = 1
            APB2 Prescaler                 = 1
            Flash Latency(WS)              = 4
            PLL_M                          = 2
            PLL_N                          = 20
            PLL_R                          = 2
	( SYSCLK = PLLCLK = VCO=(16MHz)*N = 320 /M/R = 80 MHz )
            PLL_P                          = 7 
	( PLLSAI3CLK = VCO /M/P) = 320/7 = 22.857MHz )
            PLL_Q                          = 8
	( PLL48M1CLK = VCO /Q) = 40 MHz ?! )

*/

	// SYSCLK = (IN*N)/M

	/* PLL_N=20 --- VCO=IN*PLL_N */
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN;		// set to HSI*20=320MHz
	RCC->PLLCFGR |= 20<<8;

	/* PLL_M=2 (00:M=1 01:M=2, 10:M=3, etc) */
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM;		// 320MHz/2 = 160MHz
	RCC->PLLCFGR |= 1<<4;

	/* PLL_R =2 (00=2, 01=4 10=6 11=8) */
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLR;		// 160MHz/2 = 80MHz

	/* PLL_P=7 (register=0 means 7, 1=17) for SAI1/SAI2 clock */
	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLP;
//	RCC->PLLCFGR |= RCC_PLLCFGR_PLLP;

	/* PLL_Q=8 (11) sets 48MHz clock to 320/8=40? */
//	RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLQ;
//	RCC->PLLCFGR |= 3<<21;

	RCC->CR |= RCC_CR_PLLON;
	while (!(RCC->CR & RCC_CR_PLLRDY)) ;

	/* Enable PLL clock output */
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN;

	/* Enable SAI3 clock output */
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLPEN;

	/***************************************/
	/* Select PLL as system clock source  */
	/**************************************/
	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_PLL;  /* 11: PLL used as sys clock */
	while ((RCC->CFGR & RCC_CFGR_SWS) == 0 );


	/****************************************/
	/* SAI1CLK				*/
	/****************************************/
	/* VCO = 16MHz HSI/PLLM = 8MHz		*/
	/* PLL_N=24 --- 192MHz/7=11.29MHz	*/

	/* Configure PLL */
	RCC->CR &= ~RCC_CR_PLLSAI1ON;
	while((RCC->CR & RCC_CR_PLLSAI1RDY) == RCC_CR_PLLSAI1RDY);

	RCC->PLLSAI1CFGR &= ~RCC_PLLSAI1CFGR_PLLSAI1N;
	RCC->PLLSAI1CFGR |= 24<<8;

	/* (8*24)/17 = 11.29MHz */
	RCC->PLLSAI1CFGR |= RCC_PLLSAI1CFGR_PLLSAI1P;

	/* Enable SAI clock output */
	RCC->PLLSAI1CFGR |= RCC_PLLSAI1CFGR_PLLSAI1PEN;

	/* 00 -- PLLSAI1 P clock */
	/* 01 -- PLLSAI2 P clock */
	/* 02 -- PLLSAI3 P clock */
	/* 03 -- External clock */
	RCC->CCIPR&=~RCC_CCIPR_SAI1SEL;
//	RCC->CCIPR|= 2<<22;

	RCC->CR |= RCC_CR_PLLSAI1ON;
	while (!(RCC->CR & RCC_CR_PLLSAI1RDY)) ;
}
#endif

#if 0
static void nmi_handler(void) {
	for(;;);
}

static void hardfault_handler(void) {
	for(;;);
}

extern unsigned long _etext,_data,_edata,_bss_start,_bss_end;

	/* Copy DATA and BSS segments into RAM */
void Reset_Handler(void)	{

	unsigned long *src, *dst;

	/* Copy data segment */
	/* Using linker symbols */
	src = &_etext;
	dst = &_data;
	while(dst < &_edata) *(dst++) = *(src++);

	/* Zero out the BSS */
	src = &_bss_start;
	while(src < &_bss_end) *(src++) = 0;

	/* Call main() */
	main();

}

#define STACK_LOCATION (0x20000000+(96*1024))

/* Vector Table */
uint32_t *myvectors[256]
__attribute__ ((section(".isr_vector"))) = {
	(uint32_t *) STACK_LOCATION,	/*   0:00  = stack pointer	*/  
  //(uint32_t *) Reset_Handler,	/* -15:04  = code entry point	*/
	(uint32_t *) nmi_handler,	/* -14:08  = NMI handler	*/
	(uint32_t *) hardfault_handler,	/* -13:0c = hard fault handler	*/
	(uint32_t *) nmi_handler,	/* -12:10 = MemManage		*/
	(uint32_t *) nmi_handler,	/* -11:14 = BusFault		*/
	(uint32_t *) nmi_handler,	/* -10:18 = UsageFault		*/
	(uint32_t *) nmi_handler,	/*  -9:1c = Reserved		*/
	(uint32_t *) nmi_handler,	/*  -8:20 = Reserved		*/
	(uint32_t *) nmi_handler,	/*  -7:24 = Reserved		*/
	(uint32_t *) nmi_handler,	/*  -6:28 = Reserved		*/
	(uint32_t *) nmi_handler,	/*  -5:2c = SVC Handler		*/
	(uint32_t *) nmi_handler,	/*  -4:30 = Debugmon		*/
	(uint32_t *) nmi_handler,	/*  -3:34 = Reserved		*/
	(uint32_t *) nmi_handler,	/*  -2:38 = PendSV		*/
	(uint32_t *) nmi_handler,	/*  -1:3c = SysTick		*/
	(uint32_t *) nmi_handler,	/*   0:40 = WWDG		*/
	(uint32_t *) nmi_handler,	/*   1:44 = PVD_PVM		*/
	(uint32_t *) nmi_handler,	/*   2:48 = RTC_TAMP_STAMP	*/
	(uint32_t *) nmi_handler,	/*   3:4C = RTC_WKUP		*/
	(uint32_t *) nmi_handler,	/*   4:50 = FLASH		*/
	(uint32_t *) nmi_handler,	/*   5:54 = RCC			*/
	(uint32_t *) nmi_handler,	/*   6:58 = EXTI0		*/
	(uint32_t *) nmi_handler,	/*   7:5c = EXTI1		*/
	(uint32_t *) nmi_handler,	/*   8:60 = EXTI2		*/
	(uint32_t *) nmi_handler,	/*   9:64 = EXTI3		*/
	(uint32_t *) nmi_handler,	/*  10:68 = EXTI4		*/
	(uint32_t *) nmi_handler,	/*  11:6C = DMA1_CH1		*/
	(uint32_t *) nmi_handler,	/*  12:70 = DMA1_CH2		*/
	(uint32_t *) nmi_handler,	/*  13:74 = DMA1_CH3		*/
	(uint32_t *) nmi_handler,	/*  14:78 = DMA1_CH4		*/
	(uint32_t *) nmi_handler,	/*  15:7c = DMA1_CH5		*/
	(uint32_t *) nmi_handler,	/*  16:80 = DMA1_CH6		*/
	(uint32_t *) nmi_handler,	/*  17:84 = DMA1_CH7		*/
	(uint32_t *) nmi_handler,	/*  18:84 = ADC1_2		*/
	(uint32_t *) nmi_handler,	/*  19:88 = CAN1_TX		*/
	(uint32_t *) nmi_handler,	/*  20:90 = CAN1_RX0		*/
	(uint32_t *) nmi_handler,	/*  21:94 = CAN1_RX1		*/
	(uint32_t *) nmi_handler,	/*  22:98 = CAN1_SCE		*/
	(uint32_t *) nmi_handler,	/*  23:9C = EXTI9_5		*/
	(uint32_t *) nmi_handler,	/*  24:A0 = TIM1_BRK/TIM15	*/
	(uint32_t *) nmi_handler,	/*  25:A4 = TIM1_UP/TIM16	*/
	(uint32_t *) nmi_handler,	/*  26:A8 = TIM1_TRG_COM/TIM17	*/
	(uint32_t *) nmi_handler,	/*  27:AC = TIM1_CC		*/
	(uint32_t *) nmi_handler,	/*  28:B0 = TIM2		*/
	(uint32_t *) nmi_handler,	/*  29:B4 = TIM3		*/
	(uint32_t *) nmi_handler,	/*  30:B8 = TIM4		*/
	(uint32_t *) nmi_handler,	/*  31:BC = I2C1_EV		*/
	(uint32_t *) nmi_handler,	/*  32:C0 = I2C1_ER		*/
	(uint32_t *) nmi_handler,	/*  33:C4 = I2C2_EV		*/
	(uint32_t *) nmi_handler,	/*  34:C8 = I2C2_ER		*/
	(uint32_t *) nmi_handler,	/*  35:CC = SPI1		*/
	(uint32_t *) nmi_handler,	/*  36:D0 = SPI2		*/
	(uint32_t *) nmi_handler,	/*  37:D4 = USART1		*/
	(uint32_t *) nmi_handler,	/*  38:D8 = USART2		*/
	(uint32_t *) nmi_handler,	/*  39:DC = USART3		*/
	(uint32_t *) nmi_handler,	/*  40:E0 = EXTI5_10		*/
	(uint32_t *) nmi_handler,	/*  41:E4 = RTC_ALART		*/
	(uint32_t *) nmi_handler,	/*  42:E8 = DFSDM1_FLT3		*/
	(uint32_t *) nmi_handler,	/*  43:EC = TIM8_BRK		*/
	(uint32_t *) nmi_handler,	/*  44:F0 = TIM8_UP		*/
	(uint32_t *) nmi_handler,	/*  45:F4 = TIM8_TRG_COM	*/
	(uint32_t *) nmi_handler,	/*  46:F8 = TIM8_CC		*/
	(uint32_t *) nmi_handler,	/*  47:FC = ADC3		*/
	(uint32_t *) nmi_handler,	/*  48:100 = FMC		*/
	(uint32_t *) nmi_handler,	/*  49:104 = SDMMC1		*/
	(uint32_t *) nmi_handler,	/*  50:108 = TIM5		*/
	(uint32_t *) nmi_handler,	/*  51:10C = SPI3		*/
	(uint32_t *) nmi_handler,	/*  52:110 = UART4		*/
	(uint32_t *) nmi_handler,	/*  53:114 = UART5		*/
	(uint32_t *) nmi_handler,	/*  54:118 = TIM6_DACUNDER	*/
	(uint32_t *) nmi_handler,	/*  55:11C = TIM7		*/
	(uint32_t *) nmi_handler,	/*  56:120 = DMA2_CH1		*/
	(uint32_t *) nmi_handler,	/*  57:124 = DMA2_CH2		*/
	(uint32_t *) nmi_handler,	/*  58:128 = DMA2_CH3		*/
	(uint32_t *) nmi_handler,	/*  59:12C = DMA2_CH4		*/
	(uint32_t *) nmi_handler,	/*  60:130 = DMA2_CH5		*/
	(uint32_t *) nmi_handler,	/*  61:134 = DFSDM1_FLT0	*/
	(uint32_t *) nmi_handler,	/*  62:138 = DFSDM1_FLT1	*/
	(uint32_t *) nmi_handler,	/*  63:13C = DFSDM1_FLT2	*/
	(uint32_t *) nmi_handler,	/*  64:140 = COMP		*/
	(uint32_t *) nmi_handler,	/*  65:144 = LPTIM1		*/
	(uint32_t *) nmi_handler,	/*  66:148 = LPTIM2		*/
	(uint32_t *) nmi_handler,	/*  67:14C = OTG_FS		*/
	(uint32_t *) DMA_IRQHandler,	/*  68:150 = DMA2_CH6		*/
	(uint32_t *) nmi_handler,	/*  69:154 = DMA2_CH7		*/
	(uint32_t *) nmi_handler,	/*  70:158 = LPUART1		*/
	(uint32_t *) nmi_handler,	/*  71:15C = QUADSPI		*/
	(uint32_t *) nmi_handler,	/*  72:160 = I2C3_EV		*/
	(uint32_t *) nmi_handler,	/*  73:164 = I2C3_ER		*/
	(uint32_t *) nmi_handler,	/*  74:168 = SAI1		*/
	(uint32_t *) nmi_handler,	/*  75:16C = SAI2		*/
	(uint32_t *) nmi_handler,	/*  76:170 = SWPMI1		*/
	(uint32_t *) nmi_handler,	/*  77:174 = TSC		*/
	(uint32_t *) nmi_handler,	/*  78:178 = LCD		*/
	(uint32_t *) nmi_handler,	/*  79:17C = AES		*/
	(uint32_t *) nmi_handler,	/*  80:180 = RNG		*/
	(uint32_t *) nmi_handler,	/*  81:184 = FPU		*/
};
#endif
