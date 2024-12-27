/*
  Demonstração da placa ESP32-C6-LCD-1.47 da Waveshare
*/


#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>
#include <FastLED.h>

#define TFT_CS        14
#define TFT_RST       21
#define TFT_DC        15
#define TFT_MOSI      6
#define TFT_SCLK      7
#define TFT_BL        22

#define cor565(r,g,b) ((r<<11)|(g<<5)|b)
#define FUNDO   cor565(15,31,15)
#define TEXTO   cor565(4,4,16)

uint16_t bordas[] = { cor565(2,48,2), cor565(2,4,24), cor565(24,0,0) };

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// RGB LED
#define NUM_LEDS 1
#define RGB_PIN 8
CRGB ledRGB[NUM_LEDS];


void setup() {
  Serial.begin(115200);

  FastLED.addLeds<WS2812, RGB_PIN, RGB>(ledRGB, NUM_LEDS);
  ledRGB[0] = CRGB::Blue; 
  FastLED.show();

  SPI.begin(TFT_SCLK, -1, TFT_MOSI);
  pinMode (TFT_BL, OUTPUT);
  digitalWrite (TFT_BL, HIGH);

  tft.init(172, 320);
  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(1);

  int i = 0;
  for (int x = 0; x <= 30; x += 6) {
    tft.drawRoundRect(x, x, 320-2*x, 172-2*x, 24 > x? 24-x : 2, bordas[i]);
    tft.drawRoundRect(x+1, x+1, 320-2*x-2, 172-2*x-2, 24 > (x-1)? 24-x-1 : 2, bordas[i]);
    if (++i == 3) {
      i = 0;
    }
  }

  tft.fillRect(75, 60, 164, 48, FUNDO);
  tft.setTextSize(4);
  tft.setTextColor(ST77XX_BLACK);
  tft.setCursor (84, 72);
  tft.print("DQSoft");
  tft.setTextColor(TEXTO);
  tft.setCursor (86, 70);
  tft.print("DQSoft");
}

void loop() {
}
