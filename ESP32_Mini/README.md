# ESP32 Mini-Boards

Programas usados para teste de várias placas pequenas com diversos modelos do ESP32:

* Super Mini ESP32-C3
* Super Mini ESP32-C6
* S2 Mini
* ESP32-S3 Zero

Os programas foram desenvolvidos na IDE Arduino.

Mais detalhes sobre estas placas podem ser vistas em

https://github.com/dquadros/ChineseBoards/tree/main/ESP32_Mini_Boards

# AP

Este programa roda em um ESP32 que irá se comportar como um Ponto de Acesso (AP) onde as placas irão se conectar.

No meu teste eu usei uma Franzininho WiFi conectada a uma placa Franzininho WiFi LAB01. Em princípio pode ser usado com qualquer placa ESP32 com WiFi que tenha pelo menos um botão conectado a um GPIO, acertando os defines de LED, BUZZER e BOTAO e, se for o caso, retirando o código que acessa BUZZER e BOTAO.

# Placa

Este é o programa que roda na placa a ser testada. A placa precisa ter um LED (normal ou RGB WA2812B). Os comentários mencionam a configuração para as placas listadas no começo.

Para observar a saída do programa, ligar a opção "USB CDC On Boot" no menu Tools.

