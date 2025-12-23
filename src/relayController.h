#ifndef RELAY_CONTROLLER_H
#define RELAY_CONTROLLER_H

#include <Arduino.h>
#include <cstdint>

class RelayController {
private:
    const uint8_t pin;
    bool state;

public:
    RelayController(uint8_t relayPin);
    void begin();
    void turnOn();
    void turnOff();
    void toggle();
    bool getState();
    void pulse(unsigned long duration);
};

#endif
