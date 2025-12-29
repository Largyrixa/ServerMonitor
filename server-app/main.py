from os import system
from time import sleep

import serial

port = '/dev/ttyACM0'

def desligar_server():
    # system("sudo shuttdown now")
    print("desligando...")

# Configuração da serialport: str = "/dev/ttyACM0"
ser = serial.Serial(port, baudrate=9600, timeout=1)

# Loop principal
while True:
    try:
        if ser.in_waiting:
            line: str = ser.readline().decode().strip()
            response = ""

            if not line:
                continue
            elif line.upper().strip() == "DESLIGAR":
                desligar_server()
                response = "DESLIGANDO\n"
            elif line.upper().strip() == "PING":
                # Se o servidor conseguiu ler o serial, ele está ligado
                response = "LIGADO\n"
            else:
                response = "ERRO\n"
            ser.write(response.encode())
            print("ESP32:", line)
            print("RESPOSTA:", response)
    except Exception as e:
        print(e)
    sleep(1)
