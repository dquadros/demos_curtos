# Demonstração Pico 2 W
# Cria uma rede WiFi e apresenta uma página WEB para controlar LEDs RGB
# Adaptado de https://randomnerdtutorials.com/raspberry-pi-pico-web-server-micropython/

from network import WLAN
import socket
import select

import neopixel
from machine import Pin

# LEDs RGB
rgb = neopixel.NeoPixel(Pin(20), 12)
for i in range(12):
    rgb[i] = (0, 0, 0)
rgb.write()
pos = 0

# Atualiza LEDs RGB
cores = {
    "Vermelho": (64, 0, 0),
    "Verde": (0, 64, 0),
    "Azul": (0, 0, 64),
    "Apagado": (0, 0, 0)
    }

def leds(cor, sentido):
    global rgb, pos
    delta = 1 if sentido == 'Horario' else 11
    rgb[pos] = (0, 0, 0)
    pos = (pos+delta) % 12
    rgb[pos] = cores[cor]
    rgb.write()

# Wi-Fi credentials
ssid = 'PICO2W'
password = '314159265'

# Cria a rede WIFI
ap = WLAN(WLAN.IF_AP)
ap.active(True)
ap.config(essid=ssid, password=password)


# Gera o HTML a enviar
def webpage(cor, sentido):
    html = f"""
        <!DOCTYPE html>
        <html>
        <head>
            <title>Pico 2 W Web Server</title>
            <meta name="viewport" content="width=device-width, initial-scale=1">
        </head>
        <body>
            <h1>Pico 2 W Web Server</h1>
            <h2>Cor</h2>
            <form action="./red">
                <input type="submit" value="Vermelho" />
            </form>
            <form action="./green">
                <input type="submit" value="Verde" />
            </form>
            <form action="./blue">
                <input type="submit" value="Azul" />
            </form>
            <form action="./off">
                <input type="submit" value="Apagado" />
            </form>
            <br>
            <p>Cor atual: {cor}</p>
            <h2>Sentido</h2>
            <form action="./horario">
                <input type="submit" value="Horario" />
            </form>
            <form action="./antihorario">
                <input type="submit" value="Anti-horario" />
            </form>
            <br>
            <p>sentido atual: {sentido}</p>
        </body>
        </html>
        """
    return str(html)

# Prepara o socket para receber chamadas
addr = socket.getaddrinfo('0.0.0.0', 80)[0][-1]
s = socket.socket()
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind(addr)
s.listen()
s.settimeout(0.1)

print('Escutando em 192.168.4.1')

# Inicia o estado
cor = "Apagado"
sentido = "Horario"

# Loop para tratar conexões
while True:
    leds(cor, sentido)
    try:
        conn, addr = s.accept()
    except OSError as e:
        continue
            
    print('Recebeu conexão de', addr)
    try:
        # Receive and parse the request
        request = conn.recv(1024)
        request = str(request)
        print('Conteudo da requisicao = %s' % request)

        try:
            request = request.split()[1]
            print('Request:', request)
        except IndexError:
            pass
        
        # Processa o pedido
        if request == '/red?':
            cor = "Vermelho"
        elif request == '/green?':
            cor = "Verde"
        elif request == '/blue?':
            cor = "Azul"
        elif request == '/off?':
            cor = "Apagado"
        elif request == '/horario?':
            sentido = "Horario"
        elif request == '/antihorario?':
            sentido = "Anti-horario"

        # Envia a página e fecha a conexão
        conn.send('HTTP/1.0 200 OK\r\nContent-type: text/html\r\n\r\n')
        conn.send(webpage(cor, sentido))
        conn.close()

    except OSError as e:
        conn.close()
        print('Connection closed')

