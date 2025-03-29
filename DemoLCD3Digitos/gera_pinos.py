# Script para gerar a programação dos pinos

# Configuração dos pinos do display (extraída da documentação)
# config[(disp, segto] = (com, pin)
config = {}
config[(3, 'D')] = (1, 2)
config[(2, 'D')] = (1, 4)
config[(1, 'D')] = (1, 6)
config[(3, 'C')] = (2, 1)
config[(3, 'E')] = (2, 2)
config[(2, 'C')] = (2, 3)
config[(2, 'E')] = (2, 4)
config[(1, 'C')] = (2, 5)
config[(1, 'E')] = (2, 6)
config[(3, 'B')] = (3, 1)
config[(3, 'G')] = (3, 2)
config[(2, 'B')] = (3, 3)
config[(2, 'G')] = (3, 4)
config[(1, 'B')] = (3, 5)
config[(1, 'G')] = (3, 6)
config[(3, 'A')] = (4, 1)
config[(3, 'F')] = (4, 2)
config[(2, 'A')] = (4, 3)
config[(2, 'F')] = (4, 4)
config[(1, 'A')] = (4, 5)
config[(1, 'F')] = (4, 6)

# Segmentos para cada digito
segtos = {}
segtos[0] = 'ABCDEF'
segtos[1] = 'BC'
segtos[2] = 'ABGED'
segtos[3] = 'ABGCD'
segtos[4] = 'FGBC'
segtos[5] = 'AFGCD'
segtos[6] = 'AFGCDE'
segtos[7] = 'ABC'
segtos[8] = 'ABCDEFG'
segtos[9] = 'ABCDFG'

# Cria a tabela
for pos in range(3):
    print ('{')
    for digito in range(10):
        valores = [ 0, 0, 0, 0 ]
        for s in segtos[digito]:
            com, seg = config[(pos+1, s)]
            valores[com-1] = valores[com-1] | (1 << (seg-1+16))
        print (f'    {{ 0x{valores[0]:08X}, 0x{valores[1]:08X}, 0x{valores[2]:08X}, 0x{valores[3]:08X} }}, // {digito}')
    print ('},')


