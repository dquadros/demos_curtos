# RGB LED Ring, OLED Display and LM35 Sensor Example

This is a mashup of several examples to show off the CH32V003:

* WS2813B RGB LED Ring, with 12 LEDs, is driven by DMA
* LM35 temperature sensor is read through the ADC
* 128x64 OLED Display,connected through I2C, shows the temperature

## Assembly

The LED Ring is connected to PC6 (plus GND and +5V).

The LM25 sensor is connected to PD4 (plus CND and +3.3V).

The OLED display is connected to PC1 (SDA) and PC2 (SCL), plus GND and 3.3V. 

