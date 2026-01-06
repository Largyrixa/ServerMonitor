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

## Dia 2 - 22/12/25

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

## Dia 4 - 23/12/25

### Objetivos
- Integração do bot do telegram
- Implementação das classes (refatoração)
- Automatização do script python

### Sobre o bot do telegram

Existe uma biblioteca famosa chamada [Universal Telegram Bot](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot/tree/master) que parece ser bem simples de usar. Primeiramente eu criei o bot com o BotFather e depois testei com o seguinte código, disponível no repositório da biblioteca

```cpp
#include "src/environment.h" // Para o BOT_TOKEN

#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define SERIAL_BAUD 9600UL

// tempo médio entre escaneamento de mensagens
const unsigned long BOT_MTBS = 1000;

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);
unsigned long bot_lasttime;

void setup()
{
  Serial.begin(SERIAL_BAUD);
  Serial.println();
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // relé desligado

  // Configuração do WiFi
  WiFiManager wm;
  wm.setConfigPortalTimeout(240);
  if (!wm.autoConnect("ESP32")) {
    // Reinicia a placa caso não consiga conectar
    delay(3000);
    ESP.restart();
    delay(5000);
  }

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  Serial.println("*configurado com sucesso!");
}

void loop()
{
  if (millis() - bot_lasttime > BOT_MTBS) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
      for (int i = 0; i < numNewMessages; i++) {
        bot.sendMessage(bot.messages[i].chat_id, bot.messages[i].text);
      }
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }
}
```

O código só é um echo bot, ou seja, ele responde a qualquer mensagem que você mandar com o mesmo texto, e funcionou perfeitamente.

### Sobre as implementações

A primeira implementação que eu fiz foi a do controle do relé, fiz de uma maneira mais genérica, para facilitar a implementação em relays active_high

```cpp
#include "relayController.h"

RelayController::RelayController(uint8_t pin, bool active_low) : PIN(pin),  ACTIVE_LOW(active_low), state(false)
{
    pinMode(PIN, OUTPUT);
    digitalWrite(PIN, ((ACTIVE_LOW) ? HIGH : LOW));
}

bool RelayController::getState() { return state; }

void RelayController::turnOn()
{
    digitalWrite(PIN, ((ACTIVE_LOW) ? LOW : HIGH));
    state = true;
}

void RelayController::turnOff()
{
    digitalWrite(PIN, ((ACTIVE_LOW) ? HIGH : LOW));
    state = false;
}

void RelayController::pulse(unsigned long duration_ms)
{
    turnOn();
    delay(duration_ms);
    turnOff();
    delay(10); // delay final por segurança
}
```

Para o ServerController, vou usar somente o RelayController::pulse() para ligar o server.

### Nota final do dia

Não consegui terminar os objetivos, mas acredito que automatizar o script python (adicionar ele a um contêiner ou adicionar como serviço do linux) seja uma tarefa fácil.

O que consegui fazer:
- Implementei o RelayController por completo
- Implementei o ServerController, faltam métodos para tratar os comandos do chat

```cpp
class ServerController {
private:
    ServerState state;

    const unsigned long pulse_duration_ms;

    WifiClientSecure client;
    UniversalTelegramBot bot;

    // Controlador do rele
    RelayController *relay;

public:
	// Construtor: inicia os atributos e liga o servidor, se necessário
    ServerController(RelayController *relay, const unsigned long pulse_duration_ms = 100, const String &botToken = BOT_TOKEN);
    
    ServerState getState();

    // Atualiza e salva o estado atual na memória
    bool saveState();
    // Carrega o estado atual da memória
    bool loadState();

    // Monitora o servidor e manda logs caso mude o estado
    void watch(); // Não implementado
    
    // Verifica os inputs e trata os comandos
    void handleCommands(); // Não implementado

    void powerOn();
    void powerOff();

	// Manda uma mensagem para o chat no telegram
    void sendLog(const String &msg);
};
```

## Dia 5 - 25/12/25

### Objetivos

- Encontrar uma maneira de alimentar a placa sem o USB

### Motivação

Hoje eu resolvi não testar, fazer ou refatorar nenhum código do monitor em si, mas foquei em encontrar uma fonte de alimentação para a ESP32 que não o USB conectado ao notebook. Isso porquê, se o notebook ficar sem energia, a placa desliga também e não consegue religar o servidor.

### Possibilidades

 Dentre as possibilidades que pensei, fiquei entre

1. ligar um *jumper* diretamente das portas de VBUS e GND de uma fonte de celular (as mais extremas a esquerda e direita respectivamente) no VIN e GND da ESP32, o que eu achei um tanto arriscado
2. comprar um adaptador de USB tipo B que desse os pinos de alimentação diretamente, o que é o mais limpo, seguro, porém mais caro de se fazer
3. usar uma outra placa como fonte para o circuito, o que eu escolhi

### Sobre a minha escolha

Eu tinha uma placa de uma empresa brasileira chamada [Modelix](https://www.modelix.com.br/), mais especificamente a placa Modelix 3.6. A placa satisfaz a todos as minhas necessidades
1. é alimentada via USB 5v (o mesmo de carregadores celulares)
2. tem saídas de 5v (bom para a ESP32 e resolve o problema do relé)
3. tem entrada para baterias, o que pode ser uma boa coisa no futuro

O único problema dela é o software de programação, o [Modelix System](https://www.modelix.com.br/software-de-programacao-simulador), que só tem disponibilidade para Windows, o que é um empecilho para mim.

Com isso, como estou usando a IDE do Arduino, pesquisei sobre o modelo do microcontrolador da minha placa e econtrei um site que dizia que ele era um bom substituto para o Arduino Uno e para o Arduino Duemilanove.

Logo fui testar na IDE e, usando o programador do Duemilanove, funcionou. Como o módulo relé que tenho é de 5v e a saída da ESP32 é de 3.3v, aproveitei para alimentar o módulo também e, no final usei a placa como um intermediador entre a ESP32 e o relé, além de fonte do circuito. O código que usei foi o seguinte

```cpp
#define RELAY_PIN 4
#define ESP32_PIN 3
#define ESP32_SIG digitalRead(ESP32_PIN)

void setup()
{
	pinMode(ESP32_PIN, INPUT);
	pinMode(RELAY_PIN, OUTPUT);
}

void loop()
{
	digitalWrite(RELAY_PIN, ESP32_SIG);
}
```

É um código bem simples, que simplesmente transfere o sinal da ESP32 para o módulo relé. É mais seguro, pois evita que a baixa voltagem da ESP não ligue o relé.

### Descoberta

O relé do módulo que a priori eu achei que não estava funcionando, começou a funcionar com a voltagem correta de 5v, o que demonstra que a correta alimentação pode evitar falhas no circuito.

## Dia 6 - 03/12/26

### Objetivos
- Refatorar o código do controlador do server
- Adaptar o circuito para um servo motor
### Explicação

Na semana passada, eu abri o notebook que estou usando de servidor para poder conectar o relé no botão. Porém, quando abri (depois de muitos parafusos) vi que o botão era soldado na placa, ao contrário do que eu pensei, que fosse um par de fios ligados na placa, e que para ligar o computador pela placa-mãe eu precisaria de soldar uns pontos muito pequenos, o que aumentaria o risco de erro.

### Sobre a refatoração

A minha outra ideia é comprar um servo motor que servirá para acionar mecanicamente o botão do notebook, o que reduz drasticamente a complexidade e risco do projeto.

O motor que eu escolhi foi o [SG92R](https://docs.cirkitdesigner.com/component/022ad3b3-4d33-40f1-abc1-4decda41a584/servomotor-sg92r), paguei 25 reais nele em uma loja local. O escolhi por ser um bom motor, com engrenagens em fibra de carbono, o que garante resistência e confiabilidade.

### Generalização do Código

Ao invés de fazer uma classe `ServerController` que só serve para controladores com relé, vou fazer de um jeito que a classe guarde uma função para ligar e outra para desligar, e os métodos da classe servirão apenas como *wrappers* para mandar notificações no telegram.

Exemplo de como declarar o `ServerController`  agora

```cpp
bool ligar() {
	/*
	Lógica para ligar o servidor 
	*/
}

bool desligar() {
	/*
	Lógica para desligar o servidor
	*/
}

ServerState ping() {
	/*
	Lógica de ping
	*/
}

UniversalTelegramBot bot;
ServerController(bot, ligar, desligar, ping);
```

### Adaptação do Circuito

Para colocar o servo motor no circuito, apenas conectei o PWM (geralmente o cabo laranja) na porta 4 do GPIO da ESP32 e o alimentei com uma fonte externa (no meu caso a outra placa que estou usando). Para que o motor aperte o botão, coloquei umas borrachas na ponta do braço e grudei ele com cola quente na carcaça do notebook de forma que com uma pequena rotação (aproximadamente 20º) ele aperte o botão.