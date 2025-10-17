#include "Adafruit_TinyUSB.h"

int leds [] = { LED_RED, LED_GREEN, LED_BLUE };

void setup() {
  for (uint i = 0; i < sizeof(leds)/sizeof(int); i++) {
    pinMode(leds[i], OUTPUT);
    digitalWrite(leds[i], HIGH);
  }
}

void loop() {
  for (uint i = 0; i < sizeof(leds)/sizeof(int); i++) {
    digitalWrite(leds[i], LOW);
    delay(100);
    digitalWrite(leds[i], HIGH);
    delay(1500);
  }
}
