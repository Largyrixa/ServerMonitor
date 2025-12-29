#include "src/definitions.h"
#include "src/relayController.h"
#include "src/serverController.h"

#include <WiFiManager.h>

RelayController relay(RELAY_PIN, true);
ServerController *server;

void setup() 
{
  Serial.begin(SERIAL_BAUD);
  Serial.println();
  Serial.flush();

  WiFiManager wm;
  wm.setConfigPortalTimeout(240);
  if (!wm.autoConnect("ESP32")) {
    delay(1000);
    ESP.restart();
  }

  // pinMode(2, INPUT);

  // server = new ServerController(&relay);

  relay.pulse(100);
}

void loop() 
{
  const String s = Serial.readStringUntil('\n');

  if (s == "ligar") {
    relay.turnOn();
  } else if (s == "desligar") {
    relay.turnOff();
  } else if (s == "pulso") {
    relay.pulse(500);
  }
}