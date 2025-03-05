/*
 * @file display.c
 * @author Daniel Quadros
 * @brief  Rotinas simples para display alfanumérico I2C
 *         Display usa controlador compatível com HD44780
 *         Interface I2C é feita usando PCF8574
 * @date 2022-10-05
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "lcd.h"

#define BAUD_RATE 100000   // standard 100KHz

// Seleção comando / dado
static const bool CMD = false;
static const bool DADO = true;

// Comandos do controlador 
static const byte CMD_CLS = 0x01;
static const byte CMD_DISPON = 0x0C;
static const byte CMD_POSCUR = 0x80;
static const byte CMD_FUNCTIONSET = 0x20;
static const byte LCD_4BITMODE = 0x00;
static const byte LCD_2LINE = 0x08;
static const byte LCD_5x8DOTS = 0x00;

// Conexões do PCF8574 ao display
static const byte PCF_RS = 0x01;
static const byte PCF_RW = 0x02;
static const byte PCF_E  = 0x04;
static const byte PCF_A  = 0x08;   // backlight
static const byte PCF_D4 = 0x10;
static const byte PCF_D5 = 0x20;
static const byte PCF_D6 = 0x40;
static const byte PCF_D7 = 0x80;

static i2c_inst_t *i2c_id;
static byte pcf_atual = PCF_A;  // Write, backligh ON
static byte pcf_addr = 0x27;

// Envia novo valor para o PCF8574
static void writePCF() {
    i2c_write_blocking (i2c_id, pcf_addr, &pcf_atual, 1, false);
}

// Muda o sinal RS
static void setRS(bool valor) {
    if (valor) {
        pcf_atual |= PCF_RS;
    } else {
        pcf_atual &= ~PCF_RS;
    }
    writePCF();
}

// Muda o sinal E
static void setE(bool valor) {
    if (valor) {
        pcf_atual |= PCF_E;
    } else {
        pcf_atual &= ~PCF_E;
    }
    writePCF();
}

// Muda os sinais de dado
static void setDado(byte valor) {
    pcf_atual &= ~(PCF_D4 | PCF_D5 | PCF_D6 | PCF_D7);
    if (valor & 0x1) {
        pcf_atual |= PCF_D4;
    }
    if (valor & 0x2) {
        pcf_atual |= PCF_D5;
    }
    if (valor & 0x4) {
        pcf_atual |= PCF_D6;
    }
    if (valor & 0x8) {
        pcf_atual |= PCF_D7;
    }
    writePCF();
}

// Envia um byte de dado ou comandp
static void writeByte(bool rs, byte dado) {
    setRS(rs);
    setE(true);
    setDado(dado >> 4);
    setE(false);
    setE(true);
    setDado(dado & 0x0F);
    setE(false);
}

// Iniciação do Display
void lcdInit (i2c_inst_t *i2c, byte addr, int pin_sda, int pin_scl) {
    // salva parâmetros
    i2c_id = i2c;
    if (addr) {
        pcf_addr = addr;
    }

    // inicia o I2C
    uint baud = i2c_init (i2c, BAUD_RATE);
    printf ("I2C @ %u Hz\n", baud);
    
    // configura os pinos
    gpio_set_function(pin_scl, GPIO_FUNC_I2C);
    gpio_set_function(pin_sda, GPIO_FUNC_I2C);
    gpio_pull_up(pin_scl);
    gpio_pull_up(pin_sda);
    writePCF();

    sleep_ms (100);
    writeByte (CMD, 0x03);
    sleep_ms (5);    
    writeByte (CMD, 0x03);
    sleep_ms (1);    
    writeByte (CMD, 0x03);
    sleep_ms (1);    
    writeByte (CMD, 0x02);
    sleep_ms (1);    
    writeByte (CMD, CMD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS);
    sleep_ms (1);    
    writeByte (CMD, CMD_CLS);
    sleep_ms (2);    
    writeByte (CMD, CMD_DISPON);
    sleep_ms (1);    
}

// Escreve texto no display
void lcdWrite(byte lin, byte col, char *texto) {
    byte ender = col;
    if (lin == 1) {
        ender += 0x40;
    }
    writeByte (CMD, CMD_POSCUR | ender);
    while (*texto) {
        writeByte (DADO, *texto++);
    }
}

// Limpa o display
void lcdClear(void) {
    writeByte (CMD, CMD_CLS);
    sleep_ms (2);    
}
