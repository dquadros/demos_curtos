from machine import Pin, I2C
from time import sleep

i2c = I2C(0, scl=Pin(17), sda=Pin(16), freq=100000)

# classe para controlar um módulo com 4 caracteres
class MODULO(object):
    
    # atualiza todos os segmentos
    def update(self):
        self.i2c.writeto(self.addr,  self.memoria)
    
    # seleciona a intensidade
    def set_intensidade(self, intensidade=15):
        self.i2c.writeto(self.addr,  bytearray([0xE0 + (intensidade & 0x0F)]))
    
    # construtor
    def __init__(self, i2c, addr=0x70):

        # salva os parâmetros
        self.addr = addr
        self.i2c = i2c
        
        # incia a memória dos segmentos
        self.memoria = bytearray(9)
        self.memoria[0] = 0x00 # comando para ir para o endereço inicial
        for i in range(1,9):
            self.memoria[i] = 0 # todos os segmentos apagados

        # inicia o controlador
        self.i2c.writeto(self.addr,  bytearray([0x21]))  # liga oscilador
        sleep(0.1)
        self.update()
        self.set_intensidade(15)  # intensidade maxima
        self.i2c.writeto(self.addr,  bytearray([0x81]))  # liga display

    # atualiza um caracter na memoria
    # digito: de 0 a 3
    # valor: programação para os segmentos (14 bits)
    def set_char(self, digito, valor):
        pos = 1 + 2*digito
        self.memoria[pos] = valor & 0xFF
        self.memoria[pos+1] = valor >> 8

# Gerador de caracteres
# Adaptado de https://en.wikipedia.org/wiki/Segment_display
S_A  = 0x0001
S_B  = 0x0002
S_C  = 0x0004
S_D  = 0x0008
S_E  = 0x0010
S_F  = 0x0020
S_G1 = 0x0040
S_G2 = 0x0080
S_H  = 0x0100
S_I  = 0x0200
S_J  = 0x0400
S_K  = 0x0800
S_L  = 0x1000
S_M  = 0x2000

GC = {
    ' ': 0,
    '0': S_A | S_B | S_C | S_D | S_E | S_F | S_J | S_K,
    '1': S_B | S_C,
    '2': S_A | S_B | S_E | S_D | S_G1 | S_G2,
    '3': S_A | S_B | S_C | S_D | S_G1 | S_G2,
    '4': S_F | S_G1 | S_G2 | S_B | S_C,
    '5': S_A | S_F | S_G1 | S_M | S_D,
    '6': S_A | S_C | S_D | S_E | S_F | S_G1 | S_G2,
    '7': S_A | S_B | S_C,
    '8': S_A | S_B | S_C | S_D | S_E | S_F | S_G1 | S_G2,
    '9': S_A | S_B | S_C | S_D | S_F | S_G1 | S_G2,
    'A': S_A | S_B | S_C | S_E | S_F | S_G1 | S_G2,
    'B': S_A | S_B | S_C | S_D | S_G2 | S_I | S_L,
    'C': S_A | S_D | S_E | S_F,
    'D': S_A | S_B | S_C | S_D | S_I | S_L,
    'E': S_A | S_D | S_E | S_F | S_G1 | S_G2,
    'F': S_A | S_E | S_F | S_G1 | S_G2,
    'G': S_A | S_C | S_D | S_E | S_F | S_G2,
    'H': S_B | S_C | S_E | S_F | S_G1 | S_G2,
    'I': S_A | S_D | S_I | S_L,
    'J': S_B | S_C | S_D | S_E,
    'K': S_E | S_F | S_G1 | S_J | S_M,
    'L': S_D | S_E | S_F,
    'M': S_B | S_C | S_E | S_F | S_H | S_J,
    'N': S_B | S_C | S_E | S_F | S_H | S_M,
    'O': S_A | S_B | S_C | S_D | S_E | S_F,
    'P': S_A | S_B | S_E | S_F | S_G1 | S_G2,
    'Q': S_A | S_B | S_C | S_D | S_E | S_F | S_M,
    'R': S_A | S_B | S_E | S_F | S_G1 | S_G2 | S_M,
    'S': S_A | S_F | S_G1 | S_G2 | S_C | S_D,
    'T': S_A | S_I | S_L,
    'U': S_B | S_C | S_D | S_E | S_F,
    'V': S_E | S_F | S_K | S_J,
    'W': S_B | S_C | S_E | S_F | S_K | S_M,
    'X': S_H | S_J | S_K | S_M,
    'Y': S_H | S_J | S_L,
    'Z': S_A | S_J | S_K | S_D
    }


# Classe para controlar um display com até 8 modulos
class DISPLAY(object):
    
    # construtor
    def __init__(self, i2c, addrs=[0x70]):
        self.n_modulos = len(addrs)
        self.modulos = [MODULO(i2c, addr) for addr in addrs]
        self.texto = [' '] * self.n_modulos * 4
        
    # muda a intensidade
    def set_intensidade(self, intensidade):
        for modulo in self.modulos:
            modulo.set_intensidade(intensidade)

    # atualiza todos os modulos com o texto atual
    def update(self):
        pos = 0
        for i in range(self.n_modulos):
            modulo = self.modulos[i]
            for j in range(4):
                val = GC.get(self.texto[pos], 0)
                modulo.set_char(j, val)
                pos += 1
            modulo.update()

    # define o texto a mostrar
    def set_texto(self, novo_texto):
        self.texto = (novo_texto+' '*self.n_modulos*4)[0:self.n_modulos*4]
        self.update()

    # escreve o caracter car na posicao pos (0 a n_modulos*4-1)
    def write(self, pos, car):
        self.texto[pos] = car
        modulo = self.modulos[pos // 4]
        val = GC.get(car, 0)
        modulo.set_char(pos % 4, val)
        modulo.update()

    # roda o texto para esquerda e coloca novo caracter na direita
    def shift_left(self, car):
        self.texto = self.texto[1:]+car
        self.update()

i2c = I2C(0, scl=Pin(17), sda=Pin(16), freq=100000)
display = DISPLAY(i2c)
display.set_texto("1234")
sleep(2)
display.set_texto("ABCD")
sleep(2)
display.set_texto("")
sleep(2)
for c in "ABCDEFGHIJKLMNOPQRSTUVWXYZ":
    display.shift_left(c)
    sleep(1)
