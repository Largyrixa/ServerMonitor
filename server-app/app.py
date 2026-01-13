import subprocess
import time
from flask import Flask

app = Flask(__name__)

@app.route('/desligar')
def desligar():
    subprocess.run('sudo systemctl poweroff'.split())

@app.route('/ping')
def ping():
    return 'pong'

@app.route('/do/<path:command>')
def do(command:str):
    response = ''

    linux_user = 'root'
    with open('.user', 'r') as user:
        tmp = user.read()
        if tmp and len(tmp) > 0:
            linux_user = tmp

    command = f"runuser -l {linux_user} -c '{command}'"
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

