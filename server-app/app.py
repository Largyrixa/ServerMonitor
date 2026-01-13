import subprocess
import time
import os
from flask import Flask

app = Flask(__name__)
path = os.path.abspath(__file__)
path = os.path.dirname(path)
user_path = os.path.join(path, '.user')

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
    try:
        if os.path.exists(user_path):
            with open(user_path, 'r') as f:
                content = f.read().strip()
                if content:
                    linux_user = content
    except Exception:
        pass

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

