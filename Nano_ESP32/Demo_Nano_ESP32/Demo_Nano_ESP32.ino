/*
  Controle do LED RGB da Nano ESP32 via WiFi
  Daniel Quadros, setembro/24
*/

#include <WiFi.h>
#include <WebServer.h>
#include "secrets.h"

// Controle do LED RGB
#define PIN_LEDR  14
#define PIN_LEDG  15
#define PIN_LEDB  16

// Página HTML de controle do LED
static WebServer http(80);
static const char *pagina = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>"
  "<title>LED RGB</title></head><body><h2>LED RGB</h2><form method=post>"
  "<h3>Modo:</h3>"
  "<input type='radio' id='desligado' name='modo' value='desligado' checked><label for='desligado'>Desligado</label><br>"
  "<input type='radio' id='aceso' name='modo' value='aceso'><label for='aceso'>Aceso</label><br>"
  "<input type='radio' id='pulsante' name='modo' value='pulsante'><label for='pulsante'>Pulsante</label><br>"
  "<h3>Cor:</h3>"
  "<input type='radio' id='branco' name='cor' value='branco' checked><label for='branco'>Branco</label><br>"
  "<input type='radio' id='azul' name='cor' value='azul'><label for='azul'>Azul</label><br>"
  "<input type='radio' id='verde' name='cor' value='verde'><label for='verde'>Verde</label><br>"
  "<input type='radio' id='vermelho' name='cor' value='vermelho'><label for='vermelho'>Vermelho</label><br>"
  "<input type='radio' id='amarelo' name='cor' value='amarelo'><label for='amarelo'>Amarelo</label><br>"
  "<h3>Intensidade:</h3>"
  "<input type='range' id='intensidade' name='intensidade' min='0' max='5'><br>"
  "<input type='submit' value='Execute'></form></body></html>";

// Opções para a animacao
static enum {
  APAGADO = 0,
  ACESO,
  PULSA
} animacao = ACESO;
static int pulsaR, pulsaG, pulsaB;
static int baseR = 0, baseG = 63, baseB = 63;
static int passo = 0;

// Iniciação
void setup() {
  Serial.begin(115200);
  Serial.println("\nIniciando");

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  apaga_leds();

  Serial.println("Conectando");
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    busy_led();
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connectado, IP = ");
  Serial.println(WiFi.localIP());

  acende_leds(baseR, baseG, baseB);

  http.on("/", HTTP_GET, configura);
  http.on("/", HTTP_POST, atualiza);
  http.begin();
}

// Envia a página de configuração
void configura() {
    http.setContentLength(CONTENT_LENGTH_UNKNOWN);
    http.send(200, "text/html");
    http.sendContent(pagina);
}

// Atualiza o LED conforme as seleções na página de configuração
void atualiza() {
  Serial.print("POST modo=");
  Serial.print(http.arg("modo"));
  Serial.print(" cor=");
  Serial.print(http.arg("cor"));
  Serial.print(" intensidade=");
  Serial.println(http.arg("intensidade"));

  configura();

  String cor = http.arg("cor");
  int brilho = atoi(http.arg("intensidade").c_str());
  if (cor == "branco") {
    baseR = baseG = baseB = 3 << brilho;
  } else if (cor == "azul") {
    baseR = 3 << brilho;
    baseG = 3 << brilho;
    baseB = 7 << brilho;
  } else if (cor == "verde") {
    baseR = 3 << brilho;
    baseG = 7 << brilho;
    baseB = 3 << brilho;
  } else if (cor == "vermelho") {
    baseR = 7 << brilho;
    baseG = 3 << brilho;
    baseB = 3 << brilho;
  } else if (cor == "amarelo") {
    baseR = 7 << brilho;
    baseG = 7 << brilho;
    baseB = 3 << brilho;
  }

  String modo = http.arg("modo");
  if (modo == "desligado") {
    Serial.println("Apagado");
    animacao = APAGADO;
    apaga_leds();
  } else if (modo == "aceso") {
    Serial.println("Aceso");
    animacao = ACESO;
    acende_leds(baseR, baseG, baseB);
  } else if (modo == "pulsante") {
    Serial.println("Pulsa");
    animacao = PULSA;
    pulsaR = baseR;
    pulsaG = baseG;
    pulsaB = baseB;
  }

  passo = 0;
}

// Laço principal
void loop() {
  http.handleClient();
  if (animacao == PULSA) {
    if (passo == 0) {
      // reduz luminosidade
      pulsaR--;
      pulsaG--;
      pulsaB--;
      if ((pulsaR == 0) || (pulsaG == 0) || (pulsaB == 0)) {
        passo = 1;
      }
    } else {
      // aumenta luminosidade
      pulsaR++;
      pulsaG++;
      pulsaB++;
      if ((pulsaR == baseR) || (pulsaG == baseG) || (pulsaB == baseG)) {
        passo = 0;
      }
    }
    acende_leds(pulsaR, pulsaG, pulsaB);
  }
  busy_led();
  delay(30);
}

static void apaga_leds() {
  analogWrite(PIN_LEDR, 0);
  analogWrite(PIN_LEDG, 0);
  analogWrite(PIN_LEDB, 0);
}

// Acende o LED
static void acende_leds (int corR, int corG, int corB) {
  analogWrite(PIN_LEDR, 255-corR);
  analogWrite(PIN_LEDG, 255-corG);
  analogWrite(PIN_LEDB, 255-corB);
}

// pisca led para indicar ocupado
static void busy_led() {
  static int aceso = false;
  aceso = !aceso;
  digitalWrite(LED_BUILTIN, aceso? HIGH: LOW);
}
