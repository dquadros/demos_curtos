/**
 * @file main.c
 * @author Daniel Quadros
 * @brief  Teste de uso de SNTP na Raspberry Pi Pico W
 * @date 2022-10-05
 * 
 * @copyright Copyright (c) 2022, Daniel Quadros
 * 
 */

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/i2c.h"

#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#include "secret.h"
#include "lcd.h"
#include "sntp.h"

// Conexões do display
#define I2C_ID i2c0
#define PCF_ADDR 0x3F
#define PIN_SDA 16
#define PIN_SCL 17

// Servidor NTP
#define NTP_SERVER "a.ntp.br"

// Programa Principal
int main() {
    // Inicia stdio e aguarda conectar USB
    stdio_init_all();
    sleep_ms(2000);
    printf("\nTeste SNTP\n");

    // Inicia o display
    lcdInit(I2C_ID, PCF_ADDR, PIN_SDA, PIN_SCL);
    lcdWrite(0, 0, "DQSoft");

    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return 1;
    }

    // Inicia WiFi no modo Station
    cyw43_arch_enable_sta_mode();

    // Tenta conectar
    lcdWrite(1, 0, "Conectando");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        lcdWrite(1, 0, "Erro WiFi");
        printf("Erro ao conectar WiFi\n");
        return 1;
    }
    char sIP[] = "xxx.xxx.xxx.xxx";
    strcpy (sIP, ip4addr_ntoa(netif_ip4_addr(netif_list)));
    lcdWrite(1, 0, "Conectado ");
    printf ("Conectado, IP %s\n", sIP);
    sleep_ms(2000);

    SNTPinit(NTP_SERVER, -10800);   // UTC-3
    time_t displayed = 0;
    while (true) {
        SNTPupdate();
        #if PICO_CYW43_ARCH_POLL
        cyw43_arch_poll();
        sleep_ms(1);
        #else
        sleep_ms(10);
        #endif
        if (SNTPvalid()) {
            time_t agora = SNTPtime();
            if (agora != displayed) {
                displayed = agora;
                struct tm *ptm = localtime(&agora);
                char buf[]="xx/xx xx:xx:xx";
                sprintf (buf, "%02d/%02d %02d:%02d:%02d", ptm->tm_mday, ptm->tm_mon+1,
                    ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
                lcdWrite(1,0,buf);
            }
        }
    }

    cyw43_arch_deinit();
    return 0;    
}

