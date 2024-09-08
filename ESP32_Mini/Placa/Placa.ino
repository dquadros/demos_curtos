// Demonstração das placas Mini ESP32
// Módulo cliente - Se conecta à placa principal e atualiza o LED conforme as notificações
//
// Configuração na IDE:
// ESP32-C3 Supermini: Nologo ESP32C3 Super Mini
// ESP32-C6 Supermini: DFRobot Beetle ESP32C6 ???
// ESP32-S3 Zero:      Waveshare ESP32-S3-Matrix
// ESP32-S2 Mini:      LOLIN S2 Mini
//
// Daniel Quadros - set/24

#include <WiFi.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>

// Seleção da Placa
#define ESP32C3_SUPERMINI 0
#define ESP32C6_SUPERMINI 1
#define ESP32S3_ZERO      2
#define ESP32S2_MINI      3
#define PLACA  ESP32C6_SUPERMINI


// Porta do LED, varia conforme a placa
#if PLACA == ESP32C3_SUPERMINI
const int LED = LED_BUILTIN;
const int LED_RGB = -1;
const int ACESO = LOW;
const int APAGADO = HIGH;
#elif PLACA == ESP32C6_SUPERMINI
const int LED = LED_BUILTIN;
const int LED_RGB = -1;
const int ACESO = HIGH;
const int APAGADO = LOW;
#elif PLACA == ESP32S3_ZERO
const int LED = -1;
const int LED_RGB = 21;
const int ACESO = HIGH;
const int APAGADO = LOW;
#elif PLACA == ESP32S2_MINI
const int LED = LED_BUILTIN;
const int LED_RGB = -1;
const int ACESO = HIGH;
const int APAGADO = LOW;
#endif


// Para a comunicação com a placa principal
IPAddress ipPrincipal("192.168.4.1");
WiFiUDP udp;
const int udpPort = 2222;

Adafruit_NeoPixel *ledRGB;

// Iniciacao
void setup() {
  if (LED != -1) {
    pinMode(LED, OUTPUT);
    digitalWrite(LED, APAGADO);
  }
  if (LED_RGB != -1) {
    ledRGB = new Adafruit_NeoPixel(1, LED_RGB, NEO_GRB + NEO_KHZ800);
    ledRGB->begin();
    ledRGB->clear();
  }

  Serial.begin(115200);
  delay(3000);

  Serial.println("\nConnectando ao AP");
  WiFi.begin("DQSoft", "segredo123");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connectado, IP = ");
  Serial.println(WiFi.localIP());

  udp.begin(udpPort);
  if (udp.beginPacket(ipPrincipal, udpPort) &&
      udp.write(0) &&
      udp.endPacket()) {
    Serial.println("Avisou placa principal");
  }
}

// Laco principal - trata controle do LED
void loop() {
  if (udp.parsePacket()) {
    Serial.println("Recebeu mensagem");
    bool led = udp.read() == 0x31;
    Serial.println(led);
    do {
      // ignora resto dos dados
    } while (udp.read() != -1);
    if (LED != -1) {
      digitalWrite(LED, led? ACESO : APAGADO);
    }
    if (LED_RGB != -1) {
      if (led) {
        ledRGB->setPixelColor(0, ledRGB->Color(0, 150, 0));
      } else {
        ledRGB->setPixelColor(0, ledRGB->Color(150, 0, 0));
      }
      ledRGB->show();
    }
  }
  delay(100);
}
