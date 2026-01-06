#ifndef SERVERSTATE_H
#define SERVERSTATE_H

#include <Arduino.h>

enum class ServerState : uint8_t {
    BOOTING = 0,
    ACTIVE,
    SHUTTING_DOWN,
    INACTIVE,
    ERROR,
    _COUNT
};

#endif
