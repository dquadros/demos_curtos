// Definições de acesso ao display LCD

#ifndef _LCD_H

#define _LCD_H

typedef uint8_t byte;

// Rotinas em display.c
void lcdInit(i2c_inst_t *i2c, byte addr, int pin_sda, int pin_scl);
void lcdWrite(byte lin, byte col, char *texto);
void lcdClear(void);

#endif
