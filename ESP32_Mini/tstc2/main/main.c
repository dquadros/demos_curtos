/*
 * Demonstração da placa Mini ESP32-C2 Dev Board
 * Módulo cliente - Se conecta à placa principal e atualiza o LED conforme as notificações
 *
 * Este programa foi feito a partir de um mashup de exemplo da Espressif e outros
 * 
 * Daniel Quadros - set/24 
 */

#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <esp_wifi.h>
#include <esp_system.h>
#include <esp_log.h>
#include <esp_event.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include <inttypes.h>
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include <lwip/netdb.h>
#include "driver/gpio.h"

#include "sdkconfig.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"

// LED onde será conectado um LED (ligar o catodo a terra através de um resistor de 220R ou mais)
#define LED_GPIO 7

// Semáforo para aguardar final da conexão à rede
static SemaphoreHandle_t sem_got_ip = NULL;


// Mostra informações sobre o microcontrolador
void chip_info() {
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("Este e um %s com %d core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Nao conseguiu obter o tamanho da Flash\n");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "interna" : "externa");
}

// Trata os eventos de rede (WiFi e TCP/IP)
static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_id == WIFI_EVENT_STA_START) {
        printf("Conectando WiFi....\n");
    } else if (event_id == WIFI_EVENT_STA_CONNECTED) {
        printf("WiFi conectado\n");
    } else if (event_id == IP_EVENT_STA_GOT_IP) {
        // Obteve um IP, pode iniciar comunicação
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        printf("IP: " IPSTR "\n", IP2STR(&event->ip_info.ip));
        if (sem_got_ip) {
            xSemaphoreGive(sem_got_ip);
        }
    }
}

// Dispara a conexão à rede WiFi criada pela placa principal
void wifi_connection() {
    esp_netif_init();

    sem_got_ip = xSemaphoreCreateBinary();

    esp_event_loop_create_default();

    esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation);

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t my_config = {
        .sta = {
            .ssid = "DQSoft",
            .password = "segredo123",
            .bssid_set = 0
        }
    };
    esp_wifi_set_config(ESP_IF_WIFI_STA, &my_config);

    esp_wifi_start();
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_connect();
    printf ("Disparada conexão\n");

    xSemaphoreTake(sem_got_ip, portMAX_DELAY);
}

// Tratamento da comunicação com a placa principal
static void udp_client_task(void *pvParameters)
{
    char rx_buffer[32];
    char host_ip[] = "192.168.4.1";
    int addr_family = 0;
    int ip_protocol = 0;
    const int PORT = 2222;

    while (1) {

        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(host_ip);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;

        // cria o socket
        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0) {
            printf ("Erro ao criar socket: errno %d\n", errno);
            break;
        }

        // Associa o socket à porta de comunicação
        struct sockaddr_in my_addr;
        my_addr.sin_addr.s_addr = INADDR_ANY;
        my_addr.sin_family = AF_INET;
        my_addr.sin_port = htons(PORT);
        bind(sock, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));

        // Se identifica para a placa principal
        int err = sendto(sock, " ", 1, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0) {
            printf("Erro ao enviar: errno %d\n", errno);
        } else {
            printf("Avisou placa principal\n");
        }

        // Laco principal - trata controle do LED
        while (1) {
            struct sockaddr_storage source_addr;
            socklen_t socklen = sizeof(source_addr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);
            if (len > 0) {
                printf("Recebeu mensagem\n");
                gpio_set_level(LED_GPIO, rx_buffer[0]=='1' ? 1 : 0);
            }
        }

        if (sock != -1) {
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}

// Programa Principal
void app_main(void) {
    printf("\nTeste ESP32-C2\n");

    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO, 0);

    chip_info();
    
    nvs_flash_init();

    wifi_connection();

    xTaskCreate(udp_client_task, "udp_client", 4096, NULL, 5, NULL);
}
