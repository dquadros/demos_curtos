// Demonstração das placas Mini ESP32
// Módulo AP - Cria uma rede WiFi e controla o estado do LED das placas conectadas
//
// Daniel Quadros - set/24

#include <WiFi.h>
#include <WiFiUdp.h>

//Franzininho WiFi LAB01
#define LED    33
#define BUZZER 17
#define BOTAO  3

// Estado do botão
int botao = HIGH;

// Estado do LED
bool led = false;

// Para a comunicação com as demais placas
WiFiUDP udp;
const int udpPort = 2222;

int ncli = 0;
IPAddress clientes[4];


// Iniciação - Cria o ponto de acesso
void setup() {
    pinMode(BUZZER, OUTPUT);
    digitalWrite(BUZZER, LOW);
    pinMode(BOTAO, INPUT_PULLUP);
    pinMode(LED, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    Serial.begin(115200);
    delay(3000);
    Serial.println("\nCriando AP");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("DQSoft", "segredo123");
    Serial.print("AP criado, IP = ");
    Serial.println(WiFi.softAPIP());
    udp.begin(udpPort);
}

// Laço principal - verifica apertos do botão e informa às placas conectadas
void loop() {
  // Testa se recebeu contato de alguma placa
  if (udp.parsePacket()) {
    IPAddress ip = udp.remoteIP();
    if (ncli < 4) {
      clientes[ncli++] = ip;
      Serial.print(ip.toString());
      Serial.println(" conectou.");
      avisa();  // para acertar o LED da nova placa
    }
    do {
      // ignora os dados
    } while (udp.read() == -1);
  }

  // Testa se apertou ou soltou o botão
  if (digitalRead(BOTAO) != botao) {
    botao = digitalRead(BOTAO);
    if (botao == LOW) {
      Serial.println("apertou o botão");
      led = ! led;
      digitalWrite(LED, led? HIGH : LOW);
      avisa();
    }
    delay(100); // "debounce"
  }
  delay(100);
}

// Avisa aos dispositivos conectados o novo estado do LED
void avisa() {
  for (int i = 0; i < ncli; i++) {
    if (udp.beginPacket(clientes[i], udpPort) &&
        udp.write(led? 0x30 : 0x31) &&
        udp.endPacket()) {
      Serial.print("Avisado IP ");
      Serial.println(clientes[i].toString());
    }
  }
}

