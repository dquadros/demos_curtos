from machine import Pin, I2C
from time import sleep

i2c = I2C(0, scl=Pin(17), sda=Pin(16), freq=100000)
sleep(1)
displays = i2c.scan()
print(displays)
addr = displays[0]

memoria = bytearray(9)
memoria[0] = 0x00  # comando para ir para o endereço inicial
for i in range(1,9):
    memoria[i] = 0  # todos os segmentos apagados

# Atualiza todos os segmentos
def update():
    i2c.writeto(addr,  memoria)

# Iniciação
i2c.writeto(addr,  bytearray([0x21]))  # liga oscilador
sleep(0.1)
update()
i2c.writeto(addr,  bytearray([0xEF]))  # intensidade
i2c.writeto(addr,  bytearray([0x81]))  # liga display
sleep(0.1)
while (True):
    for dig in range(4):
        pos = 1 + 2*dig
        for i in range(8):
            memoria[pos] = 1 << i
            update()
            sleep(0.5)
        memoria[pos] = 0
        for i in range(6):
            memoria[pos+1] = 1 << i
            update()
            sleep(0.5)
        memoria[pos+1] = 0

