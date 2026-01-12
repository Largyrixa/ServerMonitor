import subprocess
import time
from flask import Flask

app = Flask(__name__)

@app.route('/desligar')
def desligar():
    time_now = time.localtime()
    h, m = time_now.tm_hour, time_now.tm_min

    m += 1

    if m >= 60:
        m = 0
        h += 1
        if h >= 24:
            h = 0

    command = f"shutdown {h:02}:{m:02}"
    subprocess.run(command.split())

    return f"Desligando em 1 minuto"

@app.route('/ping')
def ping():
    return 'pong'

@app.route('/do/<path:command>')
def do(command:str):
    response = ''
    try:
        result = subprocess.run(
                command,
                shell=True,
                capture_output=True,
                text=True,
                timeout=30
                )
        response = result.stdout + result.stderr

    except subprocess.TimeoutExpired as e:
        response = (e.stdout or '') + '\n\n[ALERTA: Comando interrompido por tempo limite]'

    except Exception as e:
        response = f"Erro ao executar: {str(e)}"

    response = response.strip()
    if len(response) > 1000:
        response = response[:1000] + '\n\n... [Texto cortado pelo limite de tamanho]'

    if not response:
        response = 'Comando executado (sem retorno no terminal).'

    return response + '\n' 

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=3232)

