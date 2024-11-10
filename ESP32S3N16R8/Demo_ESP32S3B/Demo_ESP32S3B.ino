// Teste/Demonstração da Placa WeActStudio ESP32-S3-B Core Board
// (C) 2024, Daniel Quadros - MIT license
//
// Configurar na IDE Arduino:
// - Board "ESP32S3 Dev Module"
// - Flash Size "16MHz (128Mb)"
// - Partition Scheme: custom (ver partition.csv)
// - PSRAM: "OPI PSRAM"
//

#include "FS.h"
#include <LittleFS.h>

#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#include "secrets.h"
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

int status = WL_IDLE_STATUS;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, -3*60*60);

// Iniciação
void setup() {
  // Inicia serial
  Serial.begin(115200);
  Serial.println ("\n\nDemonstracao da Placa WeActStudio ESP32-S3-B\n");

  // Inicia display (buffer na PSRAM)
  size_t tot_psram = ESP.getPsramSize();
  Serial.print ("PSRAM total = ");
  Serial.println (tot_psram);
  infoPSRAM();
  Display_init();
  infoPSRAM();
  Display_str((char *) "DQSoft ESP32S3-B", 0, 0);
  Display_str((char *) "PSRAM:", 1, 0);
  char sTotPsram[10];
  itoa(tot_psram/1024, sTotPsram, 10);
  Display_str(sTotPsram, 1, 7);
  Display_str((char *) "kB", 1, 12);
  Display_update();
  Serial.println();

  // Teste do sistema de arquivos na Flash
  if (initArquivos()) {
    testaArquivos();
    Display_update();
  }

  // Connecta WiFi
  while (status != WL_CONNECTED) {
    Serial.print("Conectando a rede WiFi... ");
    status = WiFi.begin(ssid, pass);
    uint32_t timeout = millis() + 10000;
    while (millis() < timeout) {
      status = WiFi.status();
      if (status == WL_CONNECTED) {
        Serial.println("Conectado");
        break;
      }
      delay(100);
    }
    if (status == WL_CONNECTED) {
      break;
    }
    Serial.println("Falhou!");
  }
  timeClient.begin();
  while (!timeClient.update()) {
    delay(500);
  }
  Serial.println("Obteve hora");

}

// Laco eterno enquanto dure
void loop() {
  timeClient.update();
  String hora = timeClient.getFormattedTime();  // hh:mm:ss
  Display_str ((char *)hora.c_str(), 3, 0);
  Display_update ();
  delay(1000);
}

// Informa a quantidade de PSRAM disponível
void infoPSRAM() {
  Serial.print ("PSRAM disponível = ");
  Serial.println (ESP.getFreePsram());
}

// Inicia a estrutura de arquivos
bool initArquivos() {
  Display_str((char *) "Arquivos:", 2, 0);
  Display_update();
  if (LittleFS.begin(false)) {
    Serial.println("LittleFS montado");
  } else {
    Serial.println("Erro ao montar LittleFS, vai tentar formatar");
    if (LittleFS.begin(true)) {
      Serial.println("LittleFS montado apos formatacao");
    } else {
      Serial.println("Nao conseguiu montar LittleFS!");
      Display_str((char *) "ERRO", 2, 10);
      Display_update();
      return false;
    }
  }
  return true;
}

// Teste simples de escrita e leitura em arquivo
// adaptado do exemplo LITTLEFS_test
void testaArquivos() {
  const char *arq = "/DADOS";
  static uint8_t buf[512];
  size_t len = 0, i;

  Serial.println("Testando acesso a arquivos");
  apaga(arq);
  listDir("/");

  // Cria o arquivo e escreve nele
  File file = LittleFS.open(arq, FILE_WRITE);
  if (!file) {
    Serial.printf("Erro ao abrir %s\n", arq);
    Display_str((char *) "ERRO", 2, 10);
    return;
  }
  Serial.print("Escrevendo");
  memset (buf, 0x55, 512);
  uint32_t start = millis();
  for (i = 0; i < 2048; i++) {
    if ((i & 0x001F) == 0x001F) {
      Serial.print(".");
    }
    file.write(buf, 512);
  }
  Serial.println();
  len = file.size();
  file.close();
  uint32_t end = millis() - start;
  Serial.printf("Escritos %u bytes em %lu ms\r\n", 2048 * 512, end);
  listDir("/");

  // Le o arquivo
  file = LittleFS.open(arq);
  start = millis();
  i = 0;
  if (file && !file.isDirectory()) {
    len = file.size();
    size_t flen = len;
    start = millis();
    Serial.print("Lendo");
    while (len) {
      size_t toRead = len;
      if (toRead > 512) {
        toRead = 512;
      }
      file.read(buf, toRead);
      if ((i++ & 0x001F) == 0x001F) {
        Serial.print(".");
      }
      len -= toRead;
    }
    Serial.println("");
    end = millis() - start;
    Serial.printf("Lidos %u bytes em %lu ms\n", flen, end);
    file.close();
    Display_str((char *) "OK", 2, 10);
  } else {
    Serial.println("Erro ao abrir o arquivo");
    Display_str((char *) "ERRO", 2, 10);
  }
}

// Apaga arquivo se existir
void apaga(const char *arq) {
  if (LittleFS.exists(arq)) {
    if (LittleFS.remove(arq)) {
      Serial.printf ("%s apagado\n", arq);
    } else {
      Serial.printf ("Erro ao apagar %s\n", arq);
    }
  }
}

// Lista um diretorio (adaptado do exemplo LITTLEFS_test)
void listDir(const char *dirname) {
  Serial.printf("Listando: %s\r\n", dirname);

  File root = LittleFS.open(dirname);
  if (!root) {
    Serial.println("- erro ao abrir o diretorio");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" - nao eh diretorio");
    return;
  }

  File file = root.openNextFile();
  bool vazio = (file == (File) 0);
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR: ");
      Serial.println(file.name());
    } else {
      Serial.print("  ARQ: ");
      Serial.print(file.name());
      Serial.print("\tTAM: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
  if (vazio) {
    Serial.println ("  <<VAZIO>>");
  }
}

