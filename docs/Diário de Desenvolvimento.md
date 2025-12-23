## Dia 1 - 19/12/25
**Objetivo:** Pesquisar sobre como fazer o projeto e consolidar escolhas de design, como comunicação esp32-server (provavelmente pelo serial da placa). Entender como funciona o módulo relé.
### Módulo Relé e ESP32
Descobri que a esp32 usa 3.3v na gpio e na saída de alimentação, só que o módulo relé usa 5v na alimentação
Entretanto, a entrada do relé precisa de menos de **2 Volts** pra ligar e mais de **2.5 Volts** para desligar, então dá pra tentar usar uma fonte externa pro relé e a própria gpio do microcontrolador
Antes de conectar a uma fonte externa, vou fazer outro teste, tentar alimentar o relé com somente 3.3v da esp32, vamo ver o que dá.

Código de teste:
```C++
#define RELAY_PIN 4

void ligarServer()
{
	digitalWrite(RELAY_PIN, LOW); // relé ligado
	delay(1000);
	digitalWrite(RELAY_PIN, HIGH); // relé desligado
}

void setup() 
{
	pinMode(RELAY_PIN, OUTPUT);
	digitalWrite(RELAY_PIN, HIGH); // relé desligado
}

void loop() 
{
	ligarServer();
	delay(1000);
}
```

Bom, aparentemente funcionou, o brilho do led do módulo está bom e o relé está fazendo o barulho de relé (um tec), mesmo que baixo

Observação, aparentemente o módulo de **2 (dois)** relés que eu comprei tem um relé que funciona melhor que o outro, conectando ao relé número 2 eu ouvi um barulho muito mais relezistico, o que eu gostei mais que o do número 1, perfeito

Com um teste no multímetro minha suspeita foi confirmada, o relé 1 do módulo simplesmente não muda o chaveamento, enquanto o relé 2, quando ligado fica com resistência baixa entre as portas COM e NC, o que significa que o relé está ligado (de fato)

![[circuito-1.png]]
*Circuito final do dia 1*

## Dia 2 - 21/12/25

Hoje eu não tive um objetivo bem claro, e também não fiz muita coisa, mas descobri algumas coisas legais.
### WiFiManager

Um dos meus receios (mesmo que nem tenha começado o projeto) era como eu iria guardar as variáveis para conectar ao WiFi. Hoje eu descobri que existe uma biblioteca chamada WiFiManager que abre um ponto de acesso pra configuração da rede e guarda na EEPROM da placa.

Código de teste baseado em uma [página](https://arduinoecia.com.br/como-usar-o-wifimanager-com-esp32-e-esp8266/)

``` C++
#include <WiFiManager.h>

void setup() {
	Serial.begin(115200);
	delay(10);

	pinMode(2, OUTPUT);
	digitalWrite(2, HIGH);

	WiFiManager wifiManager;
	wifiManager.resetSettings();
	wifiManager.setConfigPortalTimeout(240);
	if (!wifiManager.autoConnect("ESP32")) {
		Serial.println(F("Falha na conexao. Resetar e tentar novamente..."));
		delay(3000);
		ESP.restart();
		delay(5000);
	}

	wifiManager.setDisableConfigPortal(false);
	Serial.println(F("Conectado na rede WiFi."));
	Serial.print(F("Endereco IP: "));
	Serial.println(WiFi.localIP());
}

void loop() {}
```

## Dia 3 - 22/12/25

### Objetivos
- Pesquisar sobre a comunicação da placa pelo serial
- Pesquisar se há uma maneira de armazenar o estado do servidor
- Fazer um protótipo do programa que ficará no servidor

### Sobre o serial

Pesquisei um pouco e parece que existe uma biblioteca python chamada [pyserial](https://www.pyserial.com/docs) que consegue fazer a comunicação via serial

Setup
``` bash
# No diretório ServerMonitor/server-app
python3 -m venv .venv
source .venv/bin/activate

# Instala as dependências
# Por enquanto só o pyserial
pip install -r requirements.txt
```

Encontrando o dispositivo
```python
import serial.tools.list_ports

ports = serial.tools.list_ports.comports()
for port in ports:
	print(f"{port.device}: {port.description}")
``` 

Código de teste
```python
from os import system
from time import sleep

import serial


def desligar_server():
    # system("sudo shuttdown now")
    print("desligando...") # só para teste


# Configuração da serial
port: str = "/dev/ttyACM0"
ser = serial.Serial(port, baudrate=9600, timeout=1)

# Loop principal
while True:
    if ser.in_waiting:
        line: str = ser.readline().decode().strip()
        response = ""
        if not line:
            continue
        elif line.upper().strip() == "DESLIGAR":
            desligar_server()
            response = "DESLIGANDO\n"
        else:
            response = "ERRO\n"
        ser.write(response.encode())
    sleep(1)

```

Na ESP32
```cpp
#define SERIAL_BAUD 9600UL

void setup() 
{
  Serial.begin(SERIAL_BAUD);
  delay(10);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // relé desligado
}

void loop() 
{
  Serial.println("DESLIGAR");
  delay(10000);
  Serial.println("ERRO");
  delay(10000);
}
```

Ambos os códigos funcionaram muito bem como deveriam apesar de após compilar e mandar o código para a placa o serial dar uma leve bugada, mas após um reset ele volta a funcionar perfeitamente.

### Sobre a persistência de estado

Primeiramente, por que eu quero que a esp32 guarde o estado do servidor? Bom, enquanto eu pensava sobre o projeto, eu imaginei as quedas de energia que podem acontecer, o que levaria ao desligamento tanto do servidor quanto da placa. Ao armazenar o estado, podemos ao ligar a placa retornar a esse estado passado.

Para fazer essa persistência na flash da placa, podemos usar a biblioteca `nvs_flash` da própria espressif.

Exemplo de como usar baseado [nesse site](https://www.makerhero.com/blog/armazenamento-de-dados-na-memoria-flash-do-esp32/)
```cpp
#include <nvs_flash.h>

// A chave nvs é atribuida ao valor na partição nvs
bool grava_dado_nvs(uint8_t dado, const String &chave_nvs)
{
	nvs_handle handler;
	esp_err_t err;
	
	err = nvs_flash_init_partition("nvs");
	
	if (err != ESP_OK)
		return false;
	
	err = nvs_open_from_partition("nvs", "ns_nvs", NVS_READWRITE, &handler);
	
	if (err != ESP_OK)
		return false;
		
	err = nvs_set_u8(handler, chave_nvs, dado);
	
	if (err == ESP_OK) {
		nvs_commit(handler);
		nvs_close(handler);
		return true;
	} else {
		nvs_close(handler);
		return false;
	}
}

bool le_dado_nvs(uint8_t *dado_ptr, const String &chave_nvs)
{
	nvs_handle handler;
	esp_err_t err;
	
	err = nvs_flash_init_partition("nvs");
	if (err != ESP_OK)
		return false;
		
	err = nvs_open_from_partition("nvs", "ns_nvs", NVS_READWRITE, &handler);
	
	if (err != ESP_OK)
		return false;
		
	err = nvs_get_u8(handler, chave_nvs, dado_ptr);
	
	if (err == ESP_OK) {
		nvs_close(handler);
		return true;
	} else {
		nvs_close(handler);
		return false;
	}
}
```

O *enum* que eu fiz no `definitions.h` usa o `uint8_t`, como no *snippet* acima, então não devemos ter nenhum problema.

==Obs:== quando eu for montar o programa final, vou usar uma classe que engloba essas duas funções, o código acima é só um exemplo de como usar 