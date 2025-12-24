#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#ifndef RELAY_PIN
  #define RELAY_PIN 4
#endif

enum class ServerState : uint8_t { LIGANDO, LIGADO, DESLIGANDO, DESLIGADO, ERRO };

#endif
