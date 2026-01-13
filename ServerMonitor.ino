#include "src/serverState.h"
#include "src/environment.h"
#include "src/serverController.h"

#include <WiFiManager.h>
#include <ESP32Servo.h>
#include <HTTPClient.h>

using namespace std;

// Funções de controle do servidor
bool powerServerOn();
bool powerServerOff();
ServerState pingServer();
String serverCommand(const String &);

// Cliente WiFi e bot do telegram
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// Controlador do servidor
ServerController server(bot, powerServerOn, powerServerOff, pingServer, serverCommand);

// Servo motor que aciona o botão
#define SERVO_PIN 4
Servo servo;

#define SERIAL_BAUD 115200

const String HOST_NAME = "http://"SERVER_IP":3232";

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
  servo.write(0);
}

void loop() {
  server.loop();
  delay(1000);
}

// Definição das funções de controle do servidor
bool powerServerOn() {
  // Aperta o botão devagar
  const int MAX_ANG = 15;
  for (int ang = 0; ang <= MAX_ANG; ang++) {
    servo.write(ang);
    delay(50);
  }
  delay(500);

  servo.write(0);
  delay(100);

  return true;
}

bool powerServerOff() {
  HTTPClient http;

  // Manda o comando de desligar
  http.begin(HOST_NAME + "/desligar");
  const int httpCode = http.GET(); 

  // Lê a resposta do servidor
  // para verificar o recebimento da mensagem
  if (httpCode == 200)
    return true;
  else
    return false;
}

ServerState pingServer() {
  HTTPClient http;

  // Manda o comando de ping
  http.begin(HOST_NAME + "/ping");
  const int httpCode = http.GET(); 

  // Lê a resposta do servidor
  // para verificar o recebimento da mensagem
  if (httpCode == 200)
    return ServerState::ACTIVE;
  else
    return ServerState::INACTIVE;
}

String serverCommand(const String &command) {
  HTTPClient http;

  String formated_command;

  for (int i = 0; i < command.length(); i++) {
    if (command[i] == ' ' || (i == 0 && command[0] == '/')) {
      formated_command += "%20";
    } else {
      formated_command += command[i];
    }
  }

  http.begin(HOST_NAME + "/do/" + formated_command);
  const int httpCode = http.GET();

  if (httpCode == 200)
    return http.getString();
  else
    return "ERRO: Não foi possível enviar o comando";
}
