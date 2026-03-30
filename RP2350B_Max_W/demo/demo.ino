/*
   Demonstração da placa Waveshare RP2350B Plus W

   Um display LCD colorido de 1.8″ (128×160, controlador ST7735) está ligado da seguinte forma:
   Vcc: 3,3V  GND: GND  CS: GPIO21  RST: GPIO17  RS: GPIO16  SDA: GPIO19  CLK: GPIO18

   Utiliza as bibliotecas
   - Adafruit GFX Library
   - Adafruit ST7735 and ST7789 Library
   - Arduinojson by Benoit Blanchon
*/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "secrets.h"

// Conexões do display
#define TFT_CS        21
#define TFT_RST       17
#define TFT_DC        16

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Imagem Garoa aberto (codificada com RLE - Run-Length Encoding)
int aberto[] = {
  90, 0, 2, 86, 2, 0, 1, 88, 1, 0, 1, 88, 1, 0, 1, 88, 1, 0, 1, 43
  , 2, 43, 1, 0, 1, 43, 2, 43, 1, 0, 1, 43, 2, 43, 1, 0, 1, 43, 2, 43
  , 1, 0, 1, 43, 2, 43, 1, 0, 1, 43, 2, 43, 1, 0, 1, 40, 8, 40, 1, 0
  , 1, 34, 20, 34, 1, 0, 1, 31, 26, 31, 1, 0, 1, 28, 32, 28, 1, 0, 1, 26
  , 36, 26, 1, 0, 1, 24, 40, 24, 1, 0, 1, 22, 43, 23, 1, 0, 1, 21, 46, 21
  , 1, 0, 1, 19, 45, 1, 4, 19, 1, 0, 1, 18, 2, 5, 3, 5, 5, 6, 1, 6
  , 2, 8, 2, 5, 2, 18, 1, 0, 1, 17, 2, 7, 1, 7, 3, 7, 1, 7, 1, 8
  , 1, 7, 2, 17, 1, 0, 1, 17, 1, 4, 1, 3, 2, 7, 2, 3, 5, 7, 3, 10
  , 1, 22, 1, 0, 1, 16, 2, 3, 2, 4, 1, 2, 2, 3, 2, 2, 6, 3, 1, 4
  , 3, 3, 2, 3, 1, 1, 1, 3, 2, 16, 1, 0, 1, 15, 3, 3, 3, 3, 1, 7
  , 2, 2, 6, 3, 2, 3, 3, 3, 2, 3, 1, 1, 2, 2, 3, 15, 1, 0, 1, 14
  , 1, 1, 2, 16, 3, 6, 2, 3, 1, 4, 3, 3, 2, 3, 1, 1, 2, 2, 2, 1
  , 1, 14, 1, 0, 1, 13, 1, 2, 2, 17, 2, 6, 2, 7, 4, 3, 2, 3, 1, 1
  , 2, 2, 2, 1, 2, 13, 1, 0, 1, 13, 1, 2, 2, 18, 1, 2, 6, 6, 5, 3
  , 2, 3, 1, 1, 2, 2, 2, 2, 1, 13, 1, 0, 1, 12, 1, 1, 1, 1, 2, 13
  , 2, 3, 1, 2, 6, 6, 5, 3, 2, 3, 1, 1, 2, 2, 2, 1, 1, 1, 1, 12
  , 1, 0, 1, 12, 1, 1, 1, 1, 2, 3, 3, 7, 2, 3, 1, 2, 6, 2, 2, 3
  , 4, 3, 2, 3, 1, 1, 1, 3, 2, 1, 1, 1, 1, 12, 1, 0, 1, 11, 1, 1
  , 2, 1, 2, 3, 1, 14, 1, 3, 5, 3, 1, 3, 4, 3, 2, 4, 2, 3, 2, 1
  , 2, 1, 1, 11, 1, 0, 1, 11, 1, 1, 2, 1, 2, 3, 2, 13, 1, 7, 1, 3
  , 2, 2, 4, 3, 2, 9, 1, 2, 2, 1, 1, 11, 1, 0, 1, 10, 1, 1, 1, 1
  , 4, 3, 1, 1, 1, 11, 3, 9, 3, 2, 4, 3, 1, 1, 2, 7, 4, 1, 3, 10
  , 1, 0, 1, 10, 1, 2, 8, 2, 1, 3, 29, 1, 4, 1, 2, 4, 10, 10, 1, 0
  , 1, 10, 1, 1, 1, 7, 25, 2, 10, 3, 9, 6, 3, 10, 1, 0, 1, 10, 2, 9
  , 1, 1, 2, 10, 1, 2, 1, 4, 2, 3, 6, 8, 1, 1, 1, 1, 1, 9, 2, 10
  , 1, 0, 1, 9, 2, 11, 1, 1, 1, 11, 1, 1, 1, 4, 2, 4, 4, 10, 1, 1
  , 1, 11, 2, 9, 1, 0, 1, 9, 2, 12, 1, 12, 2, 5, 2, 4, 3, 12, 1, 12
  , 2, 9, 1, 0, 1, 9, 2, 12, 1, 13, 1, 5, 2, 5, 2, 12, 1, 12, 2, 9
  , 1, 0, 1, 9, 1, 13, 1, 13, 1, 5, 2, 5, 2, 12, 1, 12, 2, 9, 1, 0
  , 1, 43, 2, 5, 1, 37, 1, 0, 1, 43, 2, 43, 1, 0, 1, 43, 2, 43, 1, 0
  , 1, 43, 2, 43, 1, 0, 1, 43, 2, 43, 1, 0, 1, 43, 2, 43, 1, 0, 1, 43
  , 2, 43, 1, 0, 1, 43, 2, 43, 1, 0, 1, 43, 2, 43, 1, 0, 1, 43, 2, 43
  , 1, 0, 1, 43, 2, 43, 1, 0, 1, 43, 2, 43, 1, 0, 1, 43, 2, 43, 1, 0
  , 1, 43, 2, 43, 1, 0, 1, 43, 2, 43, 1, 0, 1, 43, 2, 43, 1, 0, 1, 43
  , 2, 43, 1, 0, 1, 43, 2, 43, 1, 0, 1, 43, 2, 43, 1, 0, 1, 43, 2, 43
  , 1, 0, 1, 43, 2, 43, 1, 0, 1, 43, 2, 43, 1, 0, 1, 43, 2, 43, 1, 0
  , 1, 43, 2, 43, 1, 0, 1, 43, 2, 43, 1, 0, 1, 43, 2, 43, 1, 0, 1, 43
  , 2, 43, 1, 0, 1, 43, 2, 43, 1, 0, 1, 43, 2, 43, 1, 0, 1, 43, 2, 43
  , 1, 0, 1, 43, 2, 43, 1, 0, 1, 43, 2, 43, 1, 0, 1, 43, 2, 43, 1, 0
  , 1, 43, 2, 43, 1, 0, 1, 43, 2, 43, 1, 0, 1, 43, 2, 9, 2, 32, 1, 0
  , 1, 43, 2, 9, 3, 31, 1, 0, 1, 43, 2, 10, 2, 31, 1, 0, 1, 43, 2, 10
  , 2, 31, 1, 0, 1, 43, 2, 10, 2, 31, 1, 0, 1, 43, 2, 10, 2, 31, 1, 0
  , 1, 43, 3, 9, 2, 31, 1, 0, 1, 44, 2, 8, 2, 32, 1, 0, 1, 44, 3, 6
  , 3, 32, 1, 0, 1, 45, 10, 33, 1, 0, 1, 47, 6, 35, 1, 0, 1, 88, 1, 0
  , 1, 88, 1, 0, 2, 86, 2, 0, 90, 0
};

// Imagem Garoa fechado (codificada com RLE - Run-Length Encoding)
int fechado[] = {
  90, 0, 1, 88, 1, 0, 1, 88, 1, 0, 1, 88, 1, 0, 1, 88, 1, 0, 1, 13
  , 5, 3, 5, 3, 5, 3, 2, 4, 2, 4, 5, 4, 5, 7, 5, 13, 1, 0, 1, 12
  , 1, 7, 1, 7, 2, 4, 1, 2, 2, 4, 2, 3, 1, 5, 1, 3, 1, 3, 2, 5
  , 1, 4, 2, 12, 1, 0, 1, 12, 1, 7, 1, 7, 1, 5, 1, 2, 2, 4, 2, 3
  , 1, 5, 1, 3, 1, 5, 1, 4, 1, 5, 1, 12, 1, 0, 1, 12, 1, 7, 1, 7
  , 1, 8, 2, 4, 2, 3, 1, 5, 1, 3, 1, 6, 1, 2, 2, 5, 1, 12, 1, 0
  , 1, 12, 1, 7, 1, 7, 1, 8, 2, 4, 2, 3, 1, 5, 1, 3, 1, 6, 1, 2
  , 2, 5, 1, 12, 1, 0, 1, 12, 5, 3, 5, 3, 1, 8, 8, 3, 1, 5, 1, 3
  , 1, 6, 1, 3, 1, 5, 1, 12, 1, 0, 1, 12, 1, 7, 1, 7, 1, 8, 8, 3
  , 7, 3, 1, 6, 1, 3, 1, 5, 1, 12, 1, 0, 1, 12, 1, 7, 1, 7, 1, 8
  , 2, 4, 2, 3, 1, 1, 3, 1, 1, 3, 1, 6, 1, 3, 1, 5, 1, 12, 1, 0
  , 1, 12, 1, 7, 1, 7, 1, 5, 1, 2, 2, 4, 2, 3, 1, 5, 1, 3, 1, 6
  , 1, 2, 2, 5, 1, 12, 1, 0, 1, 12, 1, 7, 1, 7, 1, 5, 1, 2, 2, 4
  , 2, 3, 1, 5, 1, 3, 1, 5, 1, 10, 1, 12, 1, 0, 1, 12, 1, 7, 1, 7
  , 1, 5, 1, 2, 2, 4, 2, 3, 1, 5, 1, 3, 1, 4, 2, 4, 1, 5, 1, 12
  , 1, 0, 1, 12, 1, 7, 6, 3, 1, 3, 2, 2, 2, 4, 2, 3, 1, 5, 1, 3
  , 5, 6, 3, 2, 1, 13, 1, 0, 1, 30, 3, 4, 1, 5, 1, 1, 1, 12, 1, 12
  , 3, 14, 1, 0, 1, 42, 1, 2, 1, 42, 1, 0, 1, 42, 1, 2, 1, 42, 1, 0
  , 1, 42, 1, 2, 1, 42, 1, 0, 1, 42, 1, 1, 3, 41, 1, 0, 1, 42, 3, 1
  , 1, 41, 1, 0, 1, 41, 1, 1, 2, 1, 1, 41, 1, 0, 1, 41, 1, 1, 2, 1
  , 1, 41, 1, 0, 1, 41, 1, 1, 2, 1, 1, 41, 1, 0, 1, 41, 1, 1, 2, 1
  , 1, 41, 1, 0, 1, 41, 1, 1, 5, 40, 1, 0, 1, 41, 5, 1, 1, 40, 1, 0
  , 1, 40, 1, 1, 4, 1, 1, 40, 1, 0, 1, 40, 1, 1, 4, 1, 1, 40, 1, 0
  , 1, 40, 1, 1, 4, 1, 1, 40, 1, 0, 1, 40, 1, 1, 4, 1, 1, 40, 1, 0
  , 1, 40, 1, 1, 4, 1, 1, 40, 1, 0, 1, 40, 1, 1, 4, 1, 1, 40, 1, 0
  , 1, 40, 1, 1, 6, 40, 1, 0, 1, 40, 1, 1, 7, 39, 1, 0, 1, 40, 1, 1
  , 7, 39, 1, 0, 1, 40, 7, 1, 1, 39, 1, 0, 1, 39, 8, 1, 1, 39, 1, 0
  , 1, 39, 8, 1, 1, 39, 1, 0, 1, 39, 8, 1, 1, 39, 1, 0, 1, 39, 8, 1
  , 1, 39, 1, 0, 1, 39, 1, 1, 6, 1, 1, 39, 1, 0, 1, 39, 1, 1, 6, 1
  , 1, 39, 1, 0, 1, 39, 1, 1, 6, 1, 1, 39, 1, 0, 1, 39, 1, 1, 6, 1
  , 1, 39, 1, 0, 1, 39, 1, 1, 6, 1, 1, 39, 1, 0, 1, 39, 1, 1, 6, 1
  , 1, 39, 1, 0, 1, 39, 1, 1, 6, 1, 1, 39, 1, 0, 1, 39, 1, 1, 6, 1
  , 1, 39, 1, 0, 1, 39, 1, 1, 6, 1, 1, 39, 1, 0, 1, 39, 1, 1, 6, 1
  , 1, 39, 1, 0, 1, 39, 1, 1, 6, 1, 1, 39, 1, 0, 1, 39, 1, 1, 6, 1
  , 1, 39, 1, 0, 1, 39, 1, 1, 6, 1, 1, 39, 1, 0, 1, 39, 8, 1, 1, 39
  , 1, 0, 1, 39, 8, 1, 1, 39, 1, 0, 1, 39, 8, 1, 1, 39, 1, 0, 1, 39
  , 3, 1, 1, 2, 1, 1, 1, 39, 1, 0, 1, 40, 3, 2, 2, 1, 1, 39, 1, 0
  , 1, 40, 3, 1, 5, 39, 1, 0, 1, 40, 3, 1, 2, 1, 2, 39, 1, 0, 1, 40
  , 2, 2, 1, 1, 2, 40, 1, 0, 1, 40, 1, 3, 1, 1, 2, 40, 1, 0, 1, 40
  , 1, 3, 1, 1, 3, 39, 1, 0, 1, 40, 1, 3, 1, 1, 3, 39, 1, 0, 1, 40
  , 1, 3, 1, 1, 3, 39, 1, 0, 1, 40, 1, 3, 1, 1, 3, 39, 1, 0, 1, 44
  , 1, 1, 2, 40, 1, 0, 1, 44, 1, 1, 2, 40, 1, 0, 1, 44, 1, 1, 1, 41
  , 1, 0, 1, 44, 1, 43, 1, 0, 1, 44, 1, 43, 1, 0, 1, 44, 1, 43, 1, 0
  , 1, 44, 1, 9, 2, 32, 1, 0, 1, 44, 1, 10, 1, 32, 1, 0, 1, 44, 1, 10
  , 2, 31, 1, 0, 1, 44, 1, 10, 2, 31, 1, 0, 1, 44, 1, 10, 2, 31, 1, 0
  , 1, 44, 1, 10, 2, 31, 1, 0, 1, 44, 1, 10, 1, 32, 1, 0, 1, 44, 2, 8
  , 2, 32, 1, 0, 1, 45, 2, 6, 3, 32, 1, 0, 1, 46, 8, 34, 1, 0, 1, 48
  , 5, 35, 1, 0, 1, 88, 1, 0, 1, 88, 1, 0, 1, 88, 1, 0, 90, 0
};

// Iniciação
void setup() {
  Serial.begin(115200);
  Serial.print(F("Ola! RP2350B Plus W Demo"));

  SPI.setSCK(18);
  SPI.setTX(19);

  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST77XX_BLACK);

  splash();
  delay(3000);
  list_networks();
  delay(10000);

  connect();  
}

// Loop principal
void loop() {
  static int status_atual = -1;

  int status = check_garoa();
  if (status != status_atual) {
    status_atual = status;
    if (status_atual == 1) {
      desenha(30, aberto, sizeof(aberto)/sizeof(int), ST77XX_GREEN);
    } else if (status_atual == 0) {
      desenha(30, fechado, sizeof(fechado)/sizeof(int), ST77XX_RED);
    } else {
      tft.fillRect(0, 30, 90, 90, ST77XX_BLUE);
    }
  }
  delay(30000);
}

// Mostra a tela de apresentação
void splash() {
  tft.setTextWrap(false);
  tft.fillScreen(ST77XX_BLUE);

  tft.setCursor(0, 30);
  tft.setTextColor(ST77XX_RED);
  tft.setTextSize(3);
  tft.println("DQSoft");

  tft.setCursor(0, 80);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.println("RP2350B-Plus-W Demo");
}

// Lista as redes WiFi
void list_networks() {
  tft.fillScreen(ST77XX_BLUE);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLUE);
  tft.setTextWrap(false);
  tft.setTextSize(1);

  tft.setCursor(0, 0);
  tft.println("Scanning");

  int cnt = WiFi.scanNetworks();
  Serial.printf("Achou %d redes\n", cnt);
  tft.fillScreen(ST77XX_BLUE);
  if (cnt > 0) {
    int v = 0;
    for (int i = 0; (i < cnt) && (v < 160); i++, v += 10) {
      tft.setCursor(0, v);
      tft.print(WiFi.SSID(i));
      tft.setCursor(100, v);
      tft.print(" ");
      tft.setCursor(105, v);
      tft.print(WiFi.RSSI(i));
    }
  }
}

// Conecta à rede para acesso à internet
void connect() {
  tft.fillScreen(ST77XX_BLUE);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextWrap(false);
  tft.setTextSize(1);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ESSID, PASSWORD);
  tft.setCursor(0, 0);
  tft.println("Conectando...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  tft.setCursor(0, 10);
  tft.println("Conectado.");
}

// Desenha uma das imagens do Garoa
// As imagens estão codificadas com RLE: número de pontos em cada cor, com 0 indicando o final da linha
// Assume que o primeiro ponto de cada linha é sempre na cor de fundo
void desenha(int y, int *imagem, int tam, uint16_t color) {
  int x = 0;
  bool fundo = true;
  for (int i = 0; i < tam; i++) {
    if (imagem[i] == 0) {
      // Muda de linha
      x = 0;
      fundo = true;
      y++;
    } else {
      tft.drawFastHLine(x, y, imagem[i], fundo? ST77XX_WHITE : color);
      x += imagem[i];
      fundo = !fundo;  // alterna cor entre frente e fundo
    }
  }
}

// Acessa a API que indica se o Garoa está aberto
// GET https://garoa.net.br/status/spaceapi.json
// Retorna 0 se fechado, 1 se aberto, -1 se não conseguiu descobrir
// Adaptado do exemplo BasicHttpsClient
int check_garoa() {
  HTTPClient https;

  https.setInsecure();
  if (https.begin("https://garoa.net.br/status/spaceapi.json")) {

    int httpCode = https.GET();
    if (httpCode > 0) {
      Serial.printf("[HTTPS] GET... retornou: %d\n", httpCode);

      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = https.getString();
        Serial.print(payload);
        JsonDocument doc;
        deserializeJson(doc, payload);
        JsonObject state = doc["state"];
        bool state_open = state["open"];
        return state_open ? 1 : 0;
      }
    } else {
      Serial.printf("[HTTPS] GET... falhou, erro: %s\n", https.errorToString(httpCode).c_str());
    }

    https.end();
  } else {
    Serial.printf("[HTTPS] Nao conseguiu conectar!");
  }
  return -1;
}
