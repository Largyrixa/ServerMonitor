#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#ifndef RELAY_PIN
  #define RELAY_PIN 4
#endif

enum class ServerState : char { LIGANDO, LIGADO, DESLIGANDO, DESLIGADO };

#endif
