#ifndef RELAY_CONTROLLER_H
#define RELAY_CONTROLLER_H

#include <Arduino.h>
#include <Wire.h>

class RelayController {
private:
    const uint8_t PIN;
    const bool ACTIVE_LOW;
    bool state;

public:
    RelayController(uint8_t pin, bool active_low = true);

    void turnOn();
    void turnOff();
    bool getState();
    void pulse(unsigned long duration_ms);
};

#endif
