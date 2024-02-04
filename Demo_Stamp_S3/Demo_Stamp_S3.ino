/*
    Esta demonstração permite controlar o LED RGB da placa M5 Stamp S3 através de
    uma página WEB servida pelo próprio Stamp S3

    Requer o suporte ao esp32 by Espressif, selecionar placa STAMP-S3

    Desenvolvido com a IDE Arduino, usa as bibliotecas

    * WiFiSettings by Juerd Waalboer, Pwuts
    * SPIFFS (parte do suporte ao ESP32 by Espressif)
    * FastLED by Daniel Garcia

    O código é uma colagem destes dois exempleos:

    https://github.com/m5stack/STAMP-S3/blob/main/examples/Led/Led.ino
    https://randomnerdtutorials.com/esp32-esp8266-rgb-led-strip-web-server/

    Ignore o aviso "No hardware SPI pins defined" da FastLED.
*/

#include <WiFi.h>
#include <WiFiSettings.h>
#include <SPIFFS.h>
#include <FastLED.h>

// Conexões
#define PIN_BUTTON 0
#define PIN_LED    21
#define NUM_LEDS   1

CRGB leds[NUM_LEDS];
char localIP[20];

// Cria web server no port 80
WiFiServer server(80);

// Para decodificar os parâmetros do GET http
String redString = "0";
String greenString = "0";
String blueString = "0";
int pos1 = 0;
int pos2 = 0;
int pos3 = 0;
int pos4 = 0;

// Http request header
String header;

unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;

// Iniciação
void setup() {
  // Inicia a serial sobre a USB
  USBSerial.begin(115200);
  USBSerial.println("StampS3 demo!");

  // Inicia o botão
  pinMode(PIN_BUTTON, INPUT);

  // Prepara o controle dos LEDs
  FastLED.addLeds<WS2812, PIN_LED, GRB>(leds, NUM_LEDS);

  // Inicia WiFi
  SPIFFS.begin(true);       // Na primeira execução formata a Flash
  WiFiSettings.connect();   // Conecta à rede ou, se não encontrar cria AP p/ configurar
  strcpy (localIP, WiFi.localIP().toString().c_str());
  USBSerial.print("IP: ");
  USBSerial.println(localIP);
  server.begin();
}

// Laço principal
void loop() {
  WiFiClient client = server.available();   // Verifica conexão de um cliente

  if (client) {                             // Se um cliente conectar
    currentTime = millis();
    previousTime = currentTime;
    USBSerial.println("New Client.");
    String currentLine = "";
    // Lê o que o cliente enviou
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  
      currentTime = millis();
      if (client.available()) {             // Se tem dados a ler
        char c = client.read();                 // lê o próximo byte
        USBSerial.write(c);
        header += c;
        if (c == '\n') {                    // Se fim de linha
          // O fim da requisição são duas linhas em branco
          if (currentLine.length() == 0) {
            // Envia resposta
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
                   
            // Envia o HTML da página
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css\">");
            client.println("<script src=\"https://cdnjs.cloudflare.com/ajax/libs/jscolor/2.0.4/jscolor.min.js\"></script>");
            client.println("</head><body><div class=\"container\"><div class=\"row\"><h1>ESP Color Picker</h1></div>");
            client.println("<a class=\"btn btn-primary btn-lg\" href=\"#\" id=\"change_color\" role=\"button\">Change Color</a> ");
            client.println("<input class=\"jscolor {onFineChange:'update(this)'}\" id=\"rgb\"></div>");
            client.println("<script>function update(picker) {document.getElementById('rgb').innerHTML = Math.round(picker.rgb[0]) + ', ' +  Math.round(picker.rgb[1]) + ', ' + Math.round(picker.rgb[2]);");
            client.println("document.getElementById(\"change_color\").href=\"?r\" + Math.round(picker.rgb[0]) + \"g\" +  Math.round(picker.rgb[1]) + \"b\" + Math.round(picker.rgb[2]) + \"&\";}</script></body></html>");
            
            // Encerra com outra linha vazia
            client.println();

            // Exemplo de request: /?r201g32b255&
            // Red = 201 | Green = 32 | Blue = 255
            if(header.indexOf("GET /?r") >= 0) {
              // Extrai cores e atualiza o LED
              pos1 = header.indexOf('r');
              pos2 = header.indexOf('g');
              pos3 = header.indexOf('b');
              pos4 = header.indexOf('&');
              redString = header.substring(pos1+1, pos2);
              greenString = header.substring(pos2+1, pos3);
              blueString = header.substring(pos3+1, pos4);
              leds[0] = CRGB(redString.toInt(),
                             greenString.toInt(),
                             blueString.toInt()
                             );
              FastLED.show();
            }
            // Break out of the while loop
            break;
          } else { // Limpa linha atual ao receber fim de linha
            currentLine = "";
          }
        } else if (c != '\r') {  // coloca outras caracteres na linha atual
          currentLine += c;
        }
      }
    }
    // Limpa o header
    header = "";
    // Fecha a conexão
    client.stop();
    USBSerial.println("Client disconnected.");
    USBSerial.println("");
  }
}
