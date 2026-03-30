# Codifica imagem PNG bicolor com RLE

from PIL import Image

def isbkg(cor):
    return cor == (255,255,255)

def rasterize(arq):
    im = Image.open(arq)
    print(f'Rasterizando {arq} tamanho {im.size}')
    out = []
    for y in range(im.size[1]):
        pixel = isbkg(im.getpixel((0, y)))
        x = 0
        while x < im.size[0]:
            cont = 1
            pixel = isbkg(im.getpixel((x, y)))
            x += 1
            while (x < im.size[0]) and (pixel == isbkg(im.getpixel((x, y)))):
                cont += 1
                x += 1
            out.append(cont)
        out.append(0)
    return out


def decode(out):
    car = ' '
    for pt in out:
        if pt == 0:
            print()
        print (car*pt, end='')
        car = '*' if car == ' ' else ' '

def output(out):
    c = 0
    first = True
    print('{\n  ', end='')
    for pt in out:
        if not first:
            print(', ',end='')
        else:
            first = False
        print(pt,end='')
        c += 1
        if (c % 20) == 0:
            print('\n  ', end='')
    print('\n}')


if __name__ == "__main__":
    out = rasterize('Garoa_aberto.png')
    decode(out)
    output(out)
    out = rasterize('Garoa_fechado.png')
    output(out)

    
