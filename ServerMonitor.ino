#include "functions.h"
#include "definitions.h"

void setup() 
{
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // relé desligado
}

void loop() 
{
  LigarServer();
  delay(10000);
}