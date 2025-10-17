int leds [] = { LED_BUILTIN };

void setup() {
  for (uint i = 0; i < sizeof(leds)/sizeof(int); i++) {
    pinMode(leds[i], OUTPUT);
    digitalWrite(leds[i], HIGH);
  }
}

void loop() {
  for (uint i = 0; i < sizeof(leds)/sizeof(int); i++) {
    digitalWrite(leds[i], LOW);
    delay(300);
    digitalWrite(leds[i], HIGH);
    delay(700);
  }
}
