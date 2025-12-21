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
