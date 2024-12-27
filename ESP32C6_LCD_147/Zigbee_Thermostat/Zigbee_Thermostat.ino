// Copyright 2024 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Adaptado por Daniel Quadros em 21/12/2024 para demonstração da placa
// ESP32-C6-LCD-1.47 da Waveshare:
// - mostra informações no LCD
// - indica status através do LED RGB


/**
 * @brief This example demonstrates simple Zigbee thermostat.
 *
 * The example demonstrates how to use Zigbee library to get data from temperature
 * sensor end device and act as an thermostat.
 * The temperature sensor is a Zigbee end device, which is controlled by a Zigbee coordinator (thermostat).
 *
 * Proper Zigbee mode must be selected in Tools->Zigbee mode
 * and also the correct partition scheme must be selected in Tools->Partition Scheme.
 *
 * Please check the README.md for instructions and more detailed description.
 *
 * Created by Jan Procházka (https://github.com/P-R-O-C-H-Y/)
 */

#ifndef ZIGBEE_MODE_ZCZR
#error "Zigbee coordinator mode is not selected in Tools->Zigbee mode"
#endif

#include "Zigbee.h"

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
#define FUNDO   cor565(14,28,14)
#define BORDA   cor565(2,48,2)
//#define TEXTO   cor565(4,4,16)
#define TEXTO   cor565(0,0,0)   // Para maior contraste na gravação do vídeo

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// RGB LED
#define NUM_LEDS 1
#define RGB_PIN 8
CRGB ledRGB[NUM_LEDS];


/* Zigbee thermostat configuration */
#define THERMOSTAT_ENDPOINT_NUMBER 5
uint8_t button = BOOT_PIN;

ZigbeeThermostat zbThermostat = ZigbeeThermostat(THERMOSTAT_ENDPOINT_NUMBER);

// Save temperature sensor data
float sensor_temp;
float sensor_max_temp;
float sensor_min_temp;
float sensor_tolerance;

/****************** Temperature sensor handling *******************/
void recieveSensorTemp(float temperature) {
  Serial.printf("Temperature sensor value: %.2f°C\n", temperature);
  sensor_temp = temperature;
  char msg[20];
  sprintf (msg, "%.2fC", sensor_temp)  ;  
  msg_sensor(msg);
}

void recieveSensorConfig(float min_temp, float max_temp, float tolerance) {
  Serial.printf("Temperature sensor settings: min %.2f°C, max %.2f°C, tolerance %.2f°C\n", min_temp, max_temp, tolerance);
  sensor_min_temp = min_temp;
  sensor_max_temp = max_temp;
  sensor_tolerance = tolerance;
}

void msg_status(char *msg) {
  tft.fillRect(30, 30, 250, 20, FUNDO);
  tft.setCursor (30, 30);
  tft.setTextSize(2);
  tft.print(msg);
}

void msg_zigbee(char *msg) {
  tft.fillRect(30, 60, 250, 20, FUNDO);
  tft.setCursor (30, 60);
  tft.setTextSize(2);
  tft.print(msg);
}

void msg_sensor(char *msg) {
  tft.fillRect(30, 100, 250, 40, FUNDO);
  tft.setCursor (30, 100);
  tft.setTextSize(4);
  tft.print(msg);
}


/********************* Arduino functions **************************/
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

  tft.fillRoundRect(0, 0, 320, 172, 24, FUNDO);
  tft.drawRoundRect(0, 0, 320, 172, 24, BORDA);
  tft.drawRoundRect(4, 4, 312, 164, 20, BORDA);
  tft.setTextColor(TEXTO);
  msg_status("Starting");

  // Init button switch
  pinMode(button, INPUT_PULLUP);

  // Set callback functions for temperature and configuration receive
  zbThermostat.onTempRecieve(recieveSensorTemp);
  zbThermostat.onConfigRecieve(recieveSensorConfig);

  //Optional: set Zigbee device name and model
  zbThermostat.setManufacturerAndModel("DQSoft", "ZigbeeThermostat");

  //Add endpoint to Zigbee Core
  Zigbee.addEndpoint(&zbThermostat);

  //Open network for 300 seconds after boot
  Zigbee.setRebootOpenNetwork(300);
  msg_zigbee("Opening Network");

  // When all EPs are registered, start Zigbee with ZIGBEE_COORDINATOR mode
  if (!Zigbee.begin(ZIGBEE_COORDINATOR)) {
    ledRGB[0] = CRGB::Red; 
    FastLED.show();
    Serial.println("Zigbee failed to start!");
    Serial.println("Rebooting...");
    ESP.restart();
  }

  msg_status("Waiting for sensor");
  Serial.println("Waiting for Temperature sensor to bound to the thermostat");
  while (!zbThermostat.bound()) {
    Serial.printf(".");
    delay(500);
  }

  Serial.println();
  Serial.println("Connected");
  msg_status("Ready");
  msg_zigbee("Connected to sensor");

  ledRGB[0] = CRGB::Green; 
  FastLED.show();

  // Get temperature sensor configuration
  zbThermostat.getSensorSettings();
}

void loop() {
  // Handle button switch in loop()
  if (digitalRead(button) == LOW) {  // Push button pressed

    // Key debounce handling
    while (digitalRead(button) == LOW) {
      delay(50);
    }
    Serial.println("Set report interval");

    // Set reporting interval for temperature sensor
    zbThermostat.setTemperatureReporting(10, 30, 0.5f);
  }

  // Print temperature sensor data each 10 seconds
  static uint32_t last_print = 0;
  if (millis() - last_print > 10000) {
    last_print = millis();
    int temp_percent = (int)((sensor_temp - sensor_min_temp) / (sensor_max_temp - sensor_min_temp) * 100);
    Serial.printf("Loop temperature info: %.2f°C (%d %%)\n", sensor_temp, temp_percent);
  }
}
