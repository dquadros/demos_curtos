# Cliente SNTP para o Pico 2 W

Este repositório contém um cliente SNTP em C para a Raspberry Pi Pico 2 W e um programa principal de teste.

Este código foi criado originalmente como parte do meu estudo das funções de comunicação TCP/IP no SDK C/C++ da Raspberry Pi Pico W e recompilado em 2024 para a Pico 2 W.

O código do cliente SNTP foi adaptado de https://github.com/dquadros/ContRegressiva

O programa exige a conexão serial via USB (retirar o loop de espera por stdio_usb_connected() para não precisar disto) e um display alfanumérico de duas linhas de 16 caracteres ligado via i2C através de um CI PCF8574.
