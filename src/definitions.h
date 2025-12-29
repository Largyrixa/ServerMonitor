#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <Arduino.h>

#ifndef RELAY_PIN
  #define RELAY_PIN 4
#endif

#ifndef SERIAL_BAUD
  #define SERIAL_BAUD 9600UL
#endif

enum class ServerState : uint8_t {
    BOOTING = 0,
    ACTIVE,
    SHUTTING_DOWN,
    INACTIVE,
    ERROR,
    _COUNT
};

#endif
