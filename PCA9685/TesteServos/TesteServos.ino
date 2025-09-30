// Demonstração placa PCA9685 - Espansor I2C PWM 16 canais 
// Placa ESP32-C3 Supermini, expansor conectado aos pinos 8 (SDA) e 9 (SCL)
// Servos conectados aos canais 8 e 12
// Adaptado dos exemplos em https://github.com/NachtRaveVL/PCA9685-Arduino
//
// Daniel Quadros = set/25

// Biblioteca PCA9685 16-Channel PWM Driver Module de NachtRave
#include "PCA9685.h"

// Placa com endereço default
PCA9685 pwmController(Wire);            // Library using Wire @400kHz, and default B000000 (A5-A0) i2c address

// Parâmetros personalizados para os servos
PCA9685_ServoEval pwmServo1(144,328,554);
PCA9685_ServoEval pwmServo2(128,318,536);

// Iniciação
void setup() {

    Serial.begin(115200);               // Begin Serial and Wire interfaces
    Wire.setPins(8, 9);
    Wire.begin();

    pwmController.resetDevices();       // Resets all PCA9685 devices on i2c line
    pwmController.init();               // Initializes module using default totem-pole driver mode, and default disabled phase balancer
    pwmController.setPWMFreqServo();    // 50Hz provides standard 20ms servo phase length

}

// Laço principal
void loop() {

    Serial.println("-90");
    pwmController.setChannelPWM(8, pwmServo1.pwmForAngle(-90));
    pwmController.setChannelPWM(12, pwmServo2.pwmForAngle(-90));
    delay(1000);
    Serial.println("0");
    pwmController.setChannelPWM(8, pwmServo1.pwmForAngle(0));
    pwmController.setChannelPWM(12, pwmServo2.pwmForAngle(0));
    delay(1000);
    Serial.println("90");
    pwmController.setChannelPWM(8, pwmServo1.pwmForAngle(90));
    pwmController.setChannelPWM(12, pwmServo2.pwmForAngle(90));
    delay(1000);
    pwmController.setChannelPWM(8, pwmServo1.pwmForAngle(0));
    pwmController.setChannelPWM(12, pwmServo2.pwmForAngle(0));
    delay(1000);

}
