// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

// NOTE: CONNECT WS2812's to PC6

#define SSD1306_128X32

#include "ch32v003fun.h"
#include <stdio.h>
#include <string.h>
#include "ssd1306_i2c.h"
#include "ssd1306.h"

#define WS2812DMA_IMPLEMENTATION
//#define WSRBG //For WS2816C's.
#define WSGRB // For SK6805-EC15
#define NR_LEDS 12

#include "ws2812b_dma_spi_led_driver.h"

int led_on = -1;
uint32_t led_color = 0xFF0000;
int led_dir = 1;

// Callbacks that you must implement.
uint32_t WS2812BLEDCallback( int ledno )
{
	return (ledno == led_on)? led_color : 0;
}

/*
 * initialize adc for polling
 */
void adc_init( void )
{
	// ADCCLK = 24 MHz => RCC_ADCPRE = 0: divide by 2
	RCC->CFGR0 &= ~(0x1F<<11);
	
	// Enable GPIOD and ADC
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_ADC1;
	
	// PD4 is analog input chl 7
	GPIOD->CFGLR &= ~(0xf<<(4*4));	// CNF = 00: Analog, MODE = 00: Input
	
	// Reset the ADC to init all regs
	RCC->APB2PRSTR |= RCC_APB2Periph_ADC1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_ADC1;
	
	// Set up single conversion on chl 7
	ADC1->RSQR1 = 0;
	ADC1->RSQR2 = 0;
	ADC1->RSQR3 = 7;	// 0-9 for 8 ext inputs and two internals
	
	// set sampling time for chl 7
	ADC1->SAMPTR2 &= ~(ADC_SMP0<<(3*7));
	ADC1->SAMPTR2 |= 7<<(3*7);	// 0:7 => 3/9/15/30/43/57/73/241 cycles
		
	// turn on ADC and set rule group to sw trig
	ADC1->CTLR2 |= ADC_ADON | ADC_EXTSEL;
	
	// Reset calibration
	ADC1->CTLR2 |= ADC_RSTCAL;
	while(ADC1->CTLR2 & ADC_RSTCAL);
	
	// Calibrate
	ADC1->CTLR2 |= ADC_CAL;
	while(ADC1->CTLR2 & ADC_CAL);
	
	// should be ready for SW conversion now
}

/*
 * start conversion, wait and return result
 */
uint16_t adc_get( void )
{
	// start sw conversion (auto clears)
	ADC1->CTLR2 |= ADC_SWSTART;
	
	// wait for conversion complete
	while(!(ADC1->STATR & ADC_EOC));
	
	// get result
	return ADC1->RDATAR;
}

int main()
{
	SystemInit();
	WS2812BDMAInit( );
	ssd1306_i2c_init();
        ssd1306_init();
	adc_init();

	int n = 20;
	while(1)
	{
		if (--n == 0){
			uint32_t x = adc_get();
			uint16_t temp = (x*3300l)/1024l;
			char buf[6];
			buf[0] = (temp/100) + '0';
			buf[1] = ((temp/10)%10) + '0';
			buf[2] = ',';
			buf[3] = (temp % 10)+ '0';
			buf[4] = 'C';
			buf[5] = 0;
			ssd1306_setbuf(0);
			ssd1306_drawstr(0,0,"Temperatura:", 1);
			ssd1306_drawstr_sz(0,12,buf, 1, fontsize_16x16);
			ssd1306_refresh();
			n = 20;
		}

		while( WS2812BLEDInUse );
		Delay_Ms(50);
		led_on += led_dir;
		if (led_on == NR_LEDS) {
			led_on--;
			led_dir = -1;
			led_color = led_color >> 8;
		}
 		if  (led_on < 0){
			led_on = 0;
			led_dir = 1;
			led_color = led_color >>8;
		}
		if (led_color == 0) {
			led_color = 0xFF0000;
		}
		WS2812BDMAStart( NR_LEDS );
	}
}

