#include "src/serverState.h"
#include "src/environment.h"
#include "src/relayController.h"
#include "src/serverController.h"

#include <WiFiManager.h>
#include <ESP32Servo.h>

using namespace std;

// Funções de controle do servidor
bool powerServerOn();
bool powerServerOff();
ServerState pingServer();

// Cliente WiFi e bot do telegram
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// Controlador do servidor
ServerController server(bot, powerServerOn, powerServerOff, pingServer);

// Servo motor que aciona o botão
#define SERVO_PIN 4
Servo servo;


#define SERIAL_BAUD 115200

void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println();
  Serial.flush();

  WiFiManager wm;
  wm.setConfigPortalTimeout(240);
  if (!wm.autoConnect("ESP32")) {
    delay(1000);
    ESP.restart();
  }

  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  server.begin();
  servo.attach(SERVO_PIN);
}

void loop() {
  const String s = Serial.readStringUntil('\n');

  if (s == "ligar") {
    server.powerOn();
  } else if (s == "desligar") {
    server.powerOff();
  } else if (s == "ping") {
    auto state = server.getState();
    switch (state) {
      case ServerState::ACTIVE:
        Serial.println("Servidor Ativo");
        break;
      case ServerState::INACTIVE:
        Serial.println("Servidor Inativo");
        break;
      default:
        Serial.println("Servidor com Erro");
        break;
    }
  }
}

// Definição das funções de controle do servidor
bool powerServerOn() {
  // Aperta o botão devagar
  const int MAX_ANG = 20;
  for (int ang = 0; ang <= MAX_ANG; ang++) {
    servo.write(ang);
    delay(10);
  }
  delay(100);

  servo.write(0);
  delay(100);

  return true;
}

bool powerServerOff() {
  // Manda o comando de desligar
  Serial.println("/desligar");
  Serial.flush();
  delay(100);

  // Lê a resposta do servidor
  // para verificar o recebimento da mensagem
  const auto response = Serial.readStringUntil('\n');
  if (response == "OK")
    return true;
  else
    return false;
}

ServerState pingServer() {
  // Manda o comando de ping
  Serial.println("/ping");
  Serial.flush();
  delay(100);

  // Lê a resposta do servidor
  const String response = Serial.readStringUntil('\n');
  if (response == "OK")
    return ServerState::ACTIVE;
  else if (response == "ERRO")
    return ServerState::ERROR;
  else  // Se não recebemos mensagem, servidor desligado
    return ServerState::INACTIVE;
}
