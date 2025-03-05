/**
 * @file SNTP.c
 * @author Daniel Quadros
 * @brief  Implementação simples do SNTP (Simple Network Time Protocol)
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

#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#include "sntp.h"

#define millis() to_ms_since_boot(get_absolute_time())

// Estados
enum {
    CONSULTAR_DNS,      // Consultar o endereço do servidor NTP
    AGUARDANDO_DNS,     // Esperando resposta do DNS
    CONSULTAR_NTP,      // Consultar a hora atual
    AGUARDANDO_NTP,     // Aguardando retorno do servidor NTP
    ESPERA_PROXIMA,     // Aguardando para fazer próxima tentativa
    ERRO                // Algo deu errado
 } estado;

// Servidor NTP
static char* servidorNTP = "a.ntp.br"; 
static const int NTP_PORT = 123;
static ip_addr_t ntp_server_address;
static bool gotServerAddress = false;

// Pacote do NTP
typedef struct {
    uint8_t ctrl;       // LI/VN/Mode
    uint8_t stratum;
    uint8_t poll;
    uint8_t precision;
    uint8_t rootDelay[4];
    uint8_t rootDispersion[4];
    uint8_t refIdent[4];
    uint8_t refTimestamp[8];
    uint8_t orgTimestamp[8];
    uint8_t recTimestamp[8];
    uint8_t txmTimestamp[8];
} PKT_NTP;

static PKT_NTP pktTx;  // pacote enviado ao servidor
static PKT_NTP pktRx;  // pacore recebido do servidor

static struct udp_pcb *ntp_pcb;

// Controle dos tempos da tentativa
static const uint32_t MIN_TENTATIVA = 30000;
static const uint32_t MAX_TENTATIVA = 180000;
static uint32_t intervTentativa = MIN_TENTATIVA;
static uint32_t proxTentativa = 0;
static uint32_t toSNTP;

// Controle dos tempos de atualização
static uint32_t ultAtualizacao = 0;
static const uint32_t intervAtualizacao = 60000; // 1 minuto em milisegundos

// Timestamp local
static uint32_t timestamp = 0; 

// Ajuste para o fuso horário (UTC-3)
static const uint32_t epochUnix = 2208988800UL;
static int fusoHorario;

// Rotinas locais
static void enviaRequisicao(void);
static void ntp_dns_callback(const char *hostname, const ip_addr_t *ipaddr, void *arg);
static void ntp_recv_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
static void putUInt32 (uint8_t *p, uint32_t val);
static uint32_t getUInt32 (uint8_t *p);

// Iniciação do acesso ao SNTP
bool SNTPinit (char *server, int fuso) {
    if (server) {
        servidorNTP = server;
    }
    fusoHorario = fuso;
    memset (&pktTx, 0, sizeof(pktTx));
    pktTx.ctrl = (4 << 3) | 3;  // Versão 4, Modo 3 (client)
    estado = CONSULTAR_DNS;

    ntp_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
    if (!ntp_pcb) {
        printf("Erro ao criar pcb\n");
        return false;
    }
    udp_recv(ntp_pcb, ntp_recv_callback, NULL);
    return true;
}

// Trata atualização periódica do timestamp local
void SNTPupdate() {
    switch (estado) {
        case CONSULTAR_DNS:
            printf ("Consultando DNS\n");
            cyw43_arch_lwip_begin();
            estado = AGUARDANDO_DNS;
            int err = dns_gethostbyname(servidorNTP, &ntp_server_address, ntp_dns_callback, NULL);
            cyw43_arch_lwip_end();
            if (err == ERR_OK) {
                // Resposta imediata (cache)
                printf ("Cache\n");
                estado = CONSULTAR_NTP;
            } else if (err != ERR_INPROGRESS) {
                printf("Falha na consulta DNS\n");
                estado = gotServerAddress ? CONSULTAR_NTP : ERRO;
            }            
            break;
        case AGUARDANDO_DNS:
            break;
        case CONSULTAR_NTP:
            enviaRequisicao ();
            toSNTP = millis() + 1000;
            estado = AGUARDANDO_NTP;
            break;
        case AGUARDANDO_NTP:
            if (millis() > toSNTP) {
                printf ("Timeout na resposta NTP\n");
                estado = ERRO;
            }
            break;
        case ESPERA_PROXIMA:
            if (millis() > proxTentativa) {
              estado = CONSULTAR_DNS;
            }
            break;
        case ERRO:
            proxTentativa = millis() + intervTentativa;
            if (intervTentativa < MAX_TENTATIVA) {
              intervTentativa += intervTentativa;
            }
            estado = ESPERA_PROXIMA;
    }
}

// Envia requisição do tempo ao servidor NTP
void enviaRequisicao () {
    uint32_t tempoDesdeAtualizacao = (millis() - ultAtualizacao) / 1000UL;
    putUInt32 (pktTx.txmTimestamp, timestamp+tempoDesdeAtualizacao);
    cyw43_arch_lwip_begin();
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, sizeof(pktTx), PBUF_RAM);
    memcpy (p->payload, &pktTx, sizeof(pktTx));
    udp_sendto(ntp_pcb, p, &ntp_server_address, NTP_PORT);
    pbuf_free(p);
    cyw43_arch_lwip_end();
}

// Informa a hora local no formato Unix
time_t SNTPtime(void) {
    uint32_t tempoDesdeAtualizacao = millis() - ultAtualizacao;
    uint32_t tempoUTC = timestamp + tempoDesdeAtualizacao/1000;
    return (time_t) (tempoUTC - epochUnix + fusoHorario);
}

// Informa se tem data e hora validos
bool SNTPvalid(void) {
  return timestamp != 0;
}

// Retorno da consulta DNS
static void ntp_dns_callback(const char *hostname, const ip_addr_t *ipaddr, void *arg) {
    if (ipaddr) {
        ntp_server_address = *ipaddr;
        printf("IP do servidor NTP %s\n", ip4addr_ntoa(ipaddr));
        gotServerAddress = true;
        estado = CONSULTAR_NTP;
    } else {
        printf("Falha na consulta DNS\n");
        estado = gotServerAddress ? CONSULTAR_NTP : ERRO;
    }
}

// Resposta à solicitação NTP
static void ntp_recv_callback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {

    // Reiniciar o intervalo entre tentativas com o mínimo
    intervTentativa = MIN_TENTATIVA;

    // Copia o pacote recebido para a nossa estrutura
    pbuf_copy_partial (p, &pktRx, sizeof(pktRx), 0);
    pbuf_free(p);

    // Consistência básica
    if (((pktRx.ctrl & 0x3F) != ( (4 << 3) | 4)) ||
        ((pktRx.ctrl & 0xC0) == (3 << 6)) ||
        (pktRx.stratum == 0) ||
        (memcmp(pktRx.orgTimestamp, pktTx.txmTimestamp, 4) != 0)) {
        // Resposta incorreta
        estado = ERRO;
    } else {
      // Pega o resultado
      ultAtualizacao = millis();
      proxTentativa = ultAtualizacao + intervAtualizacao;
      timestamp = getUInt32 (pktRx.txmTimestamp);
      estado = ESPERA_PROXIMA;
    }
}

// Rotinas para mover uint32_t de/para os pacotes
void putUInt32 (uint8_t *p, uint32_t val) {
  p[0] = (uint8_t) ((val >> 24) & 0xFF);
  p[1] = (uint8_t) ((val >> 16) & 0xFF);
  p[2] = (uint8_t) ((val >> 8) & 0xFF);
  p[3] = (uint8_t) (val & 0xFF);
}

static uint32_t getUInt32 (uint8_t *p) {
  return (((uint32_t) p[0]) << 24) |
         (((uint32_t) p[1]) << 16) |
         (((uint32_t) p[2]) << 8) |
         ((uint32_t) p[3]);
}
