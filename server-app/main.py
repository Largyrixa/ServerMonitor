from time import sleep
import subprocess

import serial

port = '/dev/ttyACM0'

# Configuração da serialport: str = "/dev/ttyACM0"
ser = serial.Serial(port, baudrate=115200, timeout=1)

# Loop principal
while True:
    try:
        if ser.in_waiting:
            line: str = ser.readline().decode().strip().lower()
            response = ""

            if not line or not line.startsWith('/'):
                continue
            elif line == "/desligar":
                ser.write("OK\n".encode())
                subprocess.run(["systemctl", "poweroff"])
            elif line == "/ping":
                # Se o servidor conseguiu ler o serial, ele está ligado
                response = b"OK\n"
            elif line.startsWith("/do "):
                response = subprocess.run(line[4:].split(' '), capture_output=True)
            else:
                response = b"ERRO\n"
            ser.write(response)
            print("ESP32:", line)
            print("RESPOSTA:", response)
    except Exception as e:
        print(e)
    sleep(0.5)
