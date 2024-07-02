/*
 * Example for using SPI with 200x200 ePaper display with 12x16 font
 * Daniel Quadros, JUL-2024
 */

#define SSD1306_128X64

#include "ch32v003fun.h"
#include "font_12x16.h"

#include <stdio.h>
#include <string.h>

#define FALSE 0
#define TRUE  1

#define TIME_UPDATE 180000

// Display connections
#define EPAPER_RST_PORT GPIOC
#define EPAPER_RST_PIN 2
#define EPAPER_RST_HIGH() EPAPER_RST_PORT->BSHR = (1<<(EPAPER_RST_PIN))
#define EPAPER_RST_LOW() EPAPER_RST_PORT->BSHR = (1<<(16+EPAPER_RST_PIN))
#define EPAPER_CS_PORT GPIOC
#define EPAPER_CS_PIN 3
#define EPAPER_CS_HIGH() EPAPER_CS_PORT->BSHR = (1<<(EPAPER_CS_PIN))
#define EPAPER_CS_LOW() EPAPER_CS_PORT->BSHR = (1<<(16+EPAPER_CS_PIN))
#define EPAPER_DC_PIN 4
#define EPAPER_DC_PORT GPIOC
#define EPAPER_DC_HIGH() EPAPER_DC_PORT->BSHR = (1<<(EPAPER_DC_PIN))
#define EPAPER_DC_LOW() EPAPER_DC_PORT->BSHR = (1<<(16+EPAPER_DC_PIN))
#define EPAPER_BUSY_PORT GPIOA
#define EPAPER_BUSY_PIN 1
#define EPAPER_BUSY() ((EPAPER_BUSY_PORT->INDR & (1 <<(EPAPER_BUSY_PIN))) != 0)

// Controller Commands
#define CMD_SWRESET			 	0x12
#define CMD_DRV_OUT_CTRL 	 	0x01
#define CMD_DATA_ENTRY_MODE	 	0x11
#define CMD_SET_RAM_XADDR	 	0x44
#define CMD_SET_RAM_YADDR	 	0x45
#define CMD_BORDER_WAVE		 	0x3C
#define CMD_READ_TEMPERATURE 	0x18
#define CMD_DISP_UPD_CTL 	 	0x22
#define CMD_ACTIVE_DISP_UPD_SEQ	0x20
#define CMD_SET_RAM_XADDR_COUNT 0x4E
#define CMD_SET_RAM_YADDR_COUNT 0x4F
#define CMD_DEEP_SLEEP 			0x10
#define CMD_WRITE_RAM 			0x24
#define CMD_WRITE_REDRAM 		0x26

// Alphanumeric Screen Size
#define NLIN 12
#define NCOL 16

// Alphanumeric buffer
uint8_t screen[NLIN*NCOL];

// Flag to signal when display is powered off
uint8_t hibernating = TRUE;

// Initialize the pins connected to the display
void pin_init() {
	// Enable GPIOC, GPIOA and SPI
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1;
	
	// setup GPIO for reset, chip select, data/cmd and busy
	EPAPER_RST_PORT->CFGLR &= ~(0xf<<(4*EPAPER_RST_PIN));
	EPAPER_RST_PORT->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*EPAPER_RST_PIN);
	EPAPER_RST_HIGH();
	EPAPER_CS_PORT->CFGLR &= ~(0xf<<(4*EPAPER_CS_PIN));
	EPAPER_CS_PORT->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*EPAPER_CS_PIN);
	EPAPER_CS_HIGH();
	EPAPER_DC_PORT->CFGLR &= ~(0xf<<(4*EPAPER_DC_PIN));
	EPAPER_DC_PORT->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*EPAPER_DC_PIN);
	EPAPER_DC_LOW();
	EPAPER_BUSY_PORT->CFGLR &= ~0xf<<(4*EPAPER_BUSY_PIN);
	EPAPER_BUSY_PORT->CFGLR |= (GPIO_Speed_In | GPIO_CNF_IN_FLOATING)<<(4*EPAPER_BUSY_PIN);

	// PC5 is SCK, 10MHz Output, alt func, p-p
	GPIOC->CFGLR &= ~(0xf<<(4*5));
	GPIOC->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_PP_AF)<<(4*5);
	
	// PC6 is MOSI, 10MHz Output, alt func, p-p
	GPIOC->CFGLR &= ~(0xf<<(4*6));
	GPIOC->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_PP_AF)<<(4*6);
	
	// Configure SPI 
	SPI1->CTLR1 = 
		SPI_NSS_Soft | SPI_CPHA_2Edge | SPI_CPOL_High | SPI_DataSize_8b |
		SPI_Mode_Master | SPI_Direction_1Line_Tx | SPI_FirstBit_MSB |
		SPI_BaudRatePrescaler_16;

	// enable SPI port
	SPI1->CTLR1 |= CTLR1_SPE_Set;
}

// Wait display controller not busy
uint8_t epd_wait_busy()
{
  uint32_t timeout = 0;
  while (EPAPER_BUSY()) {
    timeout++;
    if (timeout > 40000) {
      return TRUE;
    }
    Delay_Ms(1);
  }
  return FALSE;
}

void epd_write_cmd(uint8_t cmd)
{
	EPAPER_DC_LOW();
	EPAPER_CS_LOW();

	while(!(SPI1->STATR & SPI_STATR_TXE))
		;
	SPI1->DATAR = cmd;
	while(SPI1->STATR & SPI_STATR_BSY)
		;

	EPAPER_DC_HIGH();
	EPAPER_CS_HIGH();
}

void epd_write_data(uint8_t data)
{
	EPAPER_CS_LOW();

	while(!(SPI1->STATR & SPI_STATR_TXE))
		;
	SPI1->DATAR = data;
	while(SPI1->STATR & SPI_STATR_BSY)
		;

	EPAPER_CS_HIGH();
}


// Reset the display
void epd_reset() {
	EPAPER_RST_LOW();
	Delay_Ms(50);
	EPAPER_RST_HIGH();
	Delay_Ms(50);
	hibernating = FALSE;
}

uint8_t epd_power_on() {
	epd_write_cmd(CMD_DISP_UPD_CTL);
	epd_write_data(0xf8);
	epd_write_cmd(CMD_ACTIVE_DISP_UPD_SEQ);

	return epd_wait_busy();
}

uint8_t epd_power_off(void)
{
	epd_write_cmd(CMD_DISP_UPD_CTL);
	epd_write_data(0x83);
	epd_write_cmd(CMD_ACTIVE_DISP_UPD_SEQ);
	if (epd_wait_busy()) {
		return TRUE;
	}

	epd_write_cmd(CMD_DEEP_SLEEP);
	epd_write_data(0x01);
	hibernating = 1;
	return FALSE;
}


void epd_setpos(uint16_t x, uint16_t y)
{
	uint8_t _x;
	uint16_t _y;

	_x = x / 8;
	_y = 199 - y;

	epd_write_cmd(CMD_SET_RAM_XADDR_COUNT);
	epd_write_data(_x);

	epd_write_cmd(CMD_SET_RAM_YADDR_COUNT);
	epd_write_data(_y & 0xff);
	epd_write_data((_y >> 8) & 0x01);
}

// Init display
// (values from WeAct Studio example)
uint8_t epd_init() {
	if (hibernating) {
		epd_reset();
	}
	if (epd_wait_busy()) {
		return TRUE;
	}

	epd_write_cmd(CMD_SWRESET);
	Delay_Ms(10);
	if (epd_wait_busy()) {
		return TRUE;
	}

	epd_write_cmd(CMD_DRV_OUT_CTRL);
	epd_write_data(0xC7);
	epd_write_data(0x00);
	epd_write_data(0x01);

	epd_write_cmd(CMD_DATA_ENTRY_MODE);
	epd_write_data(0x01);

	epd_write_cmd(CMD_SET_RAM_XADDR);
	epd_write_data(0x00);
	epd_write_data(0x18);

	epd_write_cmd(CMD_SET_RAM_YADDR);
	epd_write_data(0xC7);
	epd_write_data(0x00);
	epd_write_data(0x00);
	epd_write_data(0x00);

	epd_write_cmd(CMD_BORDER_WAVE);
	epd_write_data(0x05);

	epd_write_cmd(CMD_READ_TEMPERATURE);
	epd_write_data(0x80);

	epd_setpos(0,0);

	return epd_power_on();
}

// Clear alphanumeric screen
void epd_clear () {
	memset (screen, 0x20, NLIN*NCOL);	
}

// Write text in the alphanumeric screen
void epd_write (uint8_t l, uint8_t c, uint8_t *text) {
	memcpy (screen+(l*NCOL)+c, text, strlen((char *) text));
}

						// UpLeft Horiz UpRight Vertical DnLeft, DnRight
const uint8_t cline_1[] = {  0xDA, 0xC4, 0xBF,    0xB3,   0xC0,   0xD9 };
const uint8_t cline_2[] = {  0xC9, 0xCD, 0xBB,    0xBA,   0xC8,   0xBC};

// Draw a box
void epd_box (uint8_t l, uint8_t c, uint8_t nl, uint8_t nc, uint8_t duplo) {

	uint8_t *p = screen+(l*NCOL)+c;
	const uint8_t *cl = duplo? cline_2 : cline_1;
	*p++ = cl[0];
	for (int i = 0; i < (nc-2); i++) {
		*p++ = cl[1];
	}
	*p++ = cl[2];
	for (int i = 0; i < (nl-2); i++) {
		p += NCOL-nc;
		*p = cl[3];
		p += nc-1;
		*p++ = cl[3];
	}
	p += NCOL-nc;
	*p++ = cl[4];
	for (int i = 0; i < nc-2; i++) {
		*p++ = cl[1];
	}
	*p = cl[5];
}



// Send the graphic screen data
// TODO: optimize code
void epd_send_screen () {
	EPAPER_CS_LOW();

	// Send pixels corresponding to the alphanumeric screen
	for (int l = 0; l < NLIN; l++) {
		for (int lg = 0; lg < 16; lg++) {
			uint8_t *ps = screen + l*NCOL;
			for (int c = 0; c < NCOL/2; c++) {
				// 2 chars = 24 pixels = 3 bytes
				const uint8_t *p = &console_font_12x16[(*ps++ << 5)+(lg<<1)];
				uint8_t c1 = *p++;
				uint8_t c2 = *p;
				p = &console_font_12x16[(*ps++ << 5)+(lg<<1)];
				uint8_t c3 = *p++;
				uint8_t c4 = *p;
				while(!(SPI1->STATR & SPI_STATR_TXE))
					;
				SPI1->DATAR = c1;
				while(!(SPI1->STATR & SPI_STATR_TXE))
					;
				SPI1->DATAR = c2 | (c3 >> 4);
				while(!(SPI1->STATR & SPI_STATR_TXE))
					;
				SPI1->DATAR = (c3 << 4) | (c4 >> 4);
			}
			// There is one unused byte at the end of each line
			while(!(SPI1->STATR & SPI_STATR_TXE))
				;
			SPI1->DATAR = 0xFF;
		}
	}

	// Clear remaining 8 graphical lines
	for (int lg = 0; lg < 8; lg++) {
		for (int c = 0; c < 25; c++) {
			while(!(SPI1->STATR & SPI_STATR_TXE))
				;
			SPI1->DATAR = 0xFF;
		}
	}

	// Wait for last byte shifted out
	while(SPI1->STATR & SPI_STATR_BSY)
		;
	EPAPER_CS_HIGH();
}

// Update image on the display
void epd_refresh() {
	// Turnon if necessary
	if (hibernating) {
		epd_init();
	}

	// Fill RED RAM
	epd_setpos(0, 0);
	epd_write_cmd(CMD_WRITE_REDRAM);
	epd_send_screen();

	// Fill BW RAM
	epd_setpos(0, 0);
	epd_write_cmd(CMD_WRITE_RAM);
	epd_send_screen();

	// Update the display
	epd_write_cmd(CMD_DISP_UPD_CTL);
	epd_write_data(0xF4);
	epd_write_cmd(CMD_ACTIVE_DISP_UPD_SEQ);

	epd_wait_busy();
}

// Demo screen 1
void screen1 () {
		// Write alpha screen
		epd_clear();
		epd_write(1, 0,  (uint8_t *) "DQSoft 2024");
		epd_write(3, 0,  (uint8_t *) "ABCDEFGHIJKLMNOP");
		epd_write(4, 0,  (uint8_t *) "QRSTUVWXYZ012345");
		epd_write(5, 0,  (uint8_t *) "6789[](){}/?;:");
		epd_write(7, 0,  (uint8_t *) "Call me Ishmael.");
		epd_write(8, 0,  (uint8_t *) "Some years ago -");
		epd_write(9, 0,  (uint8_t *) "never mind how");
		epd_write(10, 0, (uint8_t *) "long precisely..");

		// Update epaper
		epd_refresh();

		// Wait time between updates
		Delay_Ms(TIME_UPDATE);
}

// Demo screen 2
void screen2 () {
		// Write alpha screen
		epd_clear();
		epd_write(5, 0, (uint8_t *) " /\\_/\\    /\\_/\\");
		epd_write(6, 0, (uint8_t *) "( o.o )  ( o.o )");
		epd_write(7, 0, (uint8_t *) " > ^ <    > ^ <");

		// Update epaper
		epd_refresh();

		// Wait time between updates
		Delay_Ms(TIME_UPDATE);
}


// Demo screen 3
void screen3 () {
		// Write alpha screen
		epd_clear();
		epd_box (0, 0, 12, 16, TRUE);
		epd_box (1, 1, 10,  5, FALSE);
		epd_box (1, 6, 10,  5, FALSE);
		epd_box (1, 11, 2,  2, FALSE);
		epd_box (1, 13, 2,  2, FALSE);

		// Update epaper
		epd_refresh();

		// Wait time between updates
		Delay_Ms(TIME_UPDATE);
}


int main() {
	// 48MHz external clock
	SystemInit();

	Delay_Ms( 100 );
	printf("\r\n\nEpaper example\n\r");

	// init spi and display
	pin_init();

	while (TRUE) {
		screen1();
		screen2();
		screen3();
	}

	// Stop
	printf("End of demo\n\r");
	while(1)
		;

}
