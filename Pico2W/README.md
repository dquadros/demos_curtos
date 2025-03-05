# Demonstrações Raspberry Pi Pico 2 W

## SNTP

Utiliza o WiFi para conectar à internet e obter a data e hora atuais (usando o protocolo SNTP). A data e hora são apresentadas em um display alfanumérico 2x16 ligado via I2C.

Desenvolvido com o SDK da Raspberry Pi, foi criado inicialmente para a Pico W. Esta versão tem pequenas alterações de código (a principal é enviar as mensagens do stdio para a serial ao invés da USB) e foi compilada para a Pico 2 W.

Para compilar, crie um arquivo secret.h onde é definido `WIFI_SSID` e `WIFI_PASSWORD`. 

## WiFi_RGB

Cria uma rede WiFi com SSID PICO2W e senha 314159265. Conectanto ao endereço 192.168.4.1, é apresentada uma tela que permite controlar um anel de LED ligado ao PICO2W.

Escrito em MicroPython, este código foi adaptado de  https://randomnerdtutorials.com/raspberry-pi-pico-web-server-micropython/

## BLE_TEMP

Adaptação do exemplo temp_sensor do MicroPython, este programa cria um dispositivo BLE que envia a temperatura interna do RP2350.

O dado transmitido pode ser visto usando aplicativos no celular, como o nRFConnect da Nordic Semiconductor.
