#include "src/monitor.h"
#include "src/definitions.h"

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