# Teste simples do sensor BME680

import board
import adafruit_bme680
from time import sleep
import math

# Versão customizada de adafruit_character_lcd.character_lcd_i2c
from lcd_pcf8574 import lcd_pcf8574

i2c = board.I2C()
sensor = adafruit_bme680.Adafruit_BME680_I2C(i2c)

lcd = lcd_pcf8574(i2c, addr=0x3F)
lcd.init()
lcd.backlightOn()


while True:
    
    print('Temperatura: {} degrees C'.format(sensor.temperature))
    print('Umidade: {}%'.format(sensor.humidity))
    print('Pressao: {}hPa'.format(sensor.pressure))

    # Calculo do Indice de Qualidade do Ar (IAQ)
    # Adaptação bem simplificada do código em
    #   https://github.com/thstielow/raspi-bme680-iaq
    # O cáculo mais preciso é o da biblioteca da Bosh
    #   https://www.bosch-sensortec.com/software-tools/software/bme680-software-bsec/
    #   (código fonte fechado)

    temp = sensor.temperature
    rho_max = (6.112 * 100 * math.exp((17.62 * temp)/(243.12 + temp)))/(461.52 * (temp + 273.15))
    hum_abs = sensor.humidity * 10 * rho_max

    ph_slope = 0.03 # deveria ser determinado experimentalmente nas condições de uso
    comp_gas = sensor.gas * math.exp(ph_slope * hum_abs)

    gas_ceil = 3000 # deveria ser determinado experimentalmente
    AQ = min((comp_gas / gas_ceil)**2, 1) * 100

    print('Qualidade do Ar: {} (100 = otima)'.format(AQ))

    lcd.displayClear()
    lcd.displayWrite(1, 0, f'U:{sensor.humidity:3.0f}% P:{sensor.pressure:.0f}hPa')
    lcd.displayWrite(0, 0, f'T:{sensor.temperature:3.1f}C AQ:{AQ:3.0}')
    
    print()
    sleep(2)
