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

// Adaptado por Daniel Quadros em 20/12/2024 para demonstração da placa
// ESP32-C6-LCD-1.47 da Waveshare:
// - obtem temperatura de sensor AHT10
// - mostra informações no LCD
// - indica status através do LED RGB

/**
 * @brief This example demonstrates Zigbee temperature sensor.
 *
 * The example demonstrates how to use Zigbee library to create a end device temperature sensor.
 * The temperature sensor is a Zigbee end device, which is controlled by a Zigbee coordinator.
 *
 * Proper Zigbee mode must be selected in Tools->Zigbee mode
 * and also the correct partition scheme must be selected in Tools->Partition Scheme.
 *
 * Please check the README.md for instructions and more detailed description.
 *
 * Created by Jan Procházka (https://github.com/P-R-O-C-H-Y/)
 */

#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected in Tools->Zigbee mode"
#endif

#include "Zigbee.h"

#include <Wire.h>  
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
//#define FUNDO   cor565(14,28,14)
#define FUNDO   cor565(8,16,8)
#define BORDA   cor565(2,48,2)
//#define TEXTO   cor565(4,4,16)
#define TEXTO   cor565(0,0,0)   // Para maior contraste na gravação do vídeo

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// AHT10 I2C Address
#define ADDR      0x38  

// AHT10 Commands
uint8_t cmdInit[] = { 0xE1, 0x08, 0x00 };
uint8_t cmdConv[] = { 0xAC, 0x33, 0x00 };

// RGB LED
#define NUM_LEDS 1
#define RGB_PIN 8
CRGB ledRGB[NUM_LEDS];

/* Zigbee temperature sensor configuration */
#define TEMP_SENSOR_ENDPOINT_NUMBER 10
uint8_t button = BOOT_PIN;

ZigbeeTempSensor zbTempSensor = ZigbeeTempSensor(TEMP_SENSOR_ENDPOINT_NUMBER);

/************************ Temp sensor *****************************/
static void temp_sensor_value_update(void *arg) {
  for (;;) {
    // Start conversion
    Wire.beginTransmission(ADDR);  
    Wire.write(cmdConv, sizeof(cmdConv));  
    Wire.endTransmission();

    // Wait conversion
    delay(80);

    // Get result
    uint16_t r[6];
    float temp, humid;
    Wire.requestFrom(ADDR, 6);
    for (int i = 0; i < 6; i++) {
      r[i] = Wire.read();
    }
    humid = (r[1] << 12) + (r[2] << 4) + (r[3] >> 4);
    humid = (humid / 0x100000) * 100.0;
    temp = ((r[3] & 0x0F) << 16) + (r[4] << 8) + r[5];
    temp = (temp / 0x100000) * 200.0 - 50.0;
    Serial.printf("Updated temperature sensor value to %.2f°C\r\n", temp);
    char msg[20];
    sprintf (msg, "%.2fC", temp)  ;  
    msg_sensor(msg);

    if (Zigbee.connected()) {
      zbTempSensor.setTemperature(temp);
    }
    delay(1000);
  }
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
  Wire.begin(4, 5);
  Serial.println ("\n\nStarting...");

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

  delay(5000);  // para o AHT10 iniciar

  // Check if a calibration is needed
  uint8_t status = getStatus();
  if ((status & 0x08) == 0) {
    Serial.println ("Calibrating");
    msg_sensor("Calibrating");
    Wire.beginTransmission(ADDR);  
    Wire.write(cmdInit, sizeof(cmdInit));  
    Wire.endTransmission();
    delay(10);
  }    

  // Init button switch
  pinMode(button, INPUT_PULLUP);

  // Optional: set Zigbee device name and model
  zbTempSensor.setManufacturerAndModel("DQSoft", "ZigbeeTempSensor");

  // Set minimum and maximum temperature measurement value
  zbTempSensor.setMinMaxValue(0.0f, 100.0f);

  // Optional: Set tolerance for temperature measurement in °C (lowest possible value is 0.01°C)
  zbTempSensor.setTolerance(0.1f);

  // Add endpoint to Zigbee Core
  Zigbee.addEndpoint(&zbTempSensor);

  Serial.println("Starting Zigbee...");
  msg_zigbee("Starting Zigbee");

  // When all EPs are registered, start Zigbee in End Device mode
  if (!Zigbee.begin()) {
    ledRGB[0] = CRGB::Red; 
    FastLED.show();
    Serial.println("Zigbee failed to start!");
    Serial.println("Rebooting...");
    msg_zigbee("Zigbee ERROR");
    msg_status("Restarting");
    ESP.restart();
  } else {
    Serial.println("Zigbee started successfully!");
    msg_zigbee("Zigbee started");
  }

  // Start Temperature sensor reading task
  xTaskCreate(temp_sensor_value_update, "temp_sensor_update", 2048, NULL, 10, NULL);

  Serial.println("Connecting to network");
  msg_status("Waiting for network");
  msg_zigbee("Connecting Zigbee");
  while (!Zigbee.connected()) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  msg_zigbee("Zigbee connected");

  // Set reporting interval for temperature measurement in seconds, must be called after Zigbee.begin()
  // min_interval and max_interval in seconds, delta (temp change in 0,1 °C)
  // if min = 1 and max = 0, reporting is sent only when temperature changes by delta
  // if min = 0 and max = 10, reporting is sent every 10 seconds or temperature changes by delta
  // if min = 0, max = 10 and delta = 0, reporting is sent every 10 seconds regardless of temperature change
  zbTempSensor.setReporting(1, 0, 0.2f);

  msg_status("Ready");
  ledRGB[0] = CRGB::Green; 
  FastLED.show();
}

void loop() {
  // Checking button for factory reset
  if (digitalRead(button) == LOW) {  // Push button pressed
    // Key debounce handling
    delay(100);
    int startTime = millis();
    while (digitalRead(button) == LOW) {
      delay(50);
      if ((millis() - startTime) > 3000) {
        // If key pressed for more than 3secs, factory reset Zigbee and reboot
        Serial.println("Resetting Zigbee to factory and rebooting in 1s.");
        delay(1000);
        Zigbee.factoryReset();
      }
    }
    Serial.println("Reporting temperature");
    zbTempSensor.reportTemperature();
  }
  delay(100);
}

// Read status
int8_t getStatus ()  
{  
  Wire.requestFrom(ADDR, 1);  
  return Wire.read();  
} 
