# BME680
Demostração rápida do sensor BME680, conectado via I2C.

O código foi para o CircuitPython (https://circuitpython.org) e requer as bibliotecas `adafruit_bus_device` e `adafruit_bme680`, que podem ser obtidas de https://github.com/adafruit/Adafruit_CircuitPython_Bundle.

O cálculo do índice de qualidade do ar (IAQ) é uma simplificação do descrito em  https://github.com/thstielow/raspi-bme680-iaq

O teste foi feito com uma placa Feather RP2040 com o sensor BME680 e um display LCD alfanumérico conectados no I2C.
