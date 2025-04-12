# Demonstração sensor SHT-40
import board
from time import sleep
from adafruit_bus_device import i2c_device

# Versão customizada de adafruit_character_lcd.character_lcd_i2c
from lcd_pcf8574 import lcd_pcf8574

# Classe para acesso ao sensor
class SHT40():
    
    # Calcula CRC
    def calc_crc(self, data):
        crc = 0xFF
        for b in data:
            crc ^= b
            for _ in range(8):
                crc = (crc << 1 if crc & 0x80 == 0 else (crc << 1) ^ 0x31) & 0xFF
        return crc

    # Verifica os crc em uma resposta
    # resposta é composta por grupos (dado dado crc)
    def check_crc(self, data):
        l =len(data)
        i = 0
        while i < l:
            crc = self.calc_crc(data[i:i+2])
            if crc != data[i+2]:
                return False
            i += 3
        return True

    # Envia um comando e le a resposta
    def exec(self, cmd, delay=0.01, resplen=6):
        self.bufTx[0] = cmd
        bufRx = bytearray(resplen)
        with self._device as bus_device:
            for _ in range(3):
                if resplen == 0:
                    try:
                        bus_device.write(self.bufTx)
                        sleep(delay)
                        return bufRx
                    except Exception as e:
                        print('Erro I2C: ',e)
                        pass
                else:
                    try:
                        bus_device.write(self.bufTx)
                        sleep(delay)
                        bus_device.readinto(bufRx)
                        if self.check_crc(bufRx):
                            return bufRx
                        print('Erro CRC')
                    except Exception as e:
                        print('Erro I2C: ',e)
                        pass
        return None                    
            
    
    # construtor
    def __init__(self, i2c):
        self._device = i2c_device.I2CDevice(i2c, 0x44)
        self.bufTx = bytearray(1)
        self.exec(0x94, resplen=0)

    # Lê ID do sensor
    def read_id(self):
        bufId = self.exec(0x89)
        if bufId is None:
            return 0
        return (bufId[0] << 24) + (bufId[1] << 16) + (bufId[3] << 8) + bufId[4]

    # Le temperatura e umidade
    def read(self):
        buf = self.exec(0xFD)
        if buf is None:
            return (0, -1)
        t_ticks = (buf[0]<<8) + buf[1]
        rh_ticks = (buf[3]<<8) + buf[4]
        t_degC = -45 + 175 * t_ticks/65535
        rh_pRH = min(max(-6 + 125 * rh_ticks/65535, 0), 100)
        return (t_degC, rh_pRH)

i2c = board.I2C()

while not i2c.try_lock():
    pass
try:
    print("Enderecos I2C:", [hex(device_address) for device_address in i2c.scan()])
finally:
    i2c.unlock()

sensor = SHT40(i2c)
print(f'ID = 0x{sensor.read_id():08X}')

lcd = lcd_pcf8574(i2c, addr=0x3F)
lcd.init()
lcd.backlightOn()

while (True):
    temp, umid = sensor.read()
    if umid != -1:
        print (f'Temperatura: {temp:.2f}C  Umidade: {umid:.2f}%')
        lcd.displayClear()
        lcd.displayWrite(0, 0, f'Umidade:{umid:3.0f}%')
        lcd.displayWrite(1, 0, f'   Temp:{temp:3.1f}C')
    sleep(2)
