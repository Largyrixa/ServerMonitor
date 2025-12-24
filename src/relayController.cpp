#include "relayController.h"

RelayController::RelayController(uint8_t pin, bool active_low) :PIN(pin),  ACTIVE_LOW(active_low), state(false)
{
    pinMode(PIN, OUTPUT);
    digitalWrite(PIN, ((ACTIVE_LOW) ? HIGH : LOW));
}

bool RelayController::getState() { return state; }

void RelayController::turnOn()
{
    digitalWrite(PIN, ((ACTIVE_LOW) ? LOW : HIGH));
    state = true;
}

void RelayController::turnOff()
{
    digitalWrite(PIN, ((ACTIVE_LOW) ? HIGH : LOW));
    state = false;
}

void RelayController::pulse(unsigned long duration_ms)
{
    turnOn();
    delay(duration_ms);
    turnOff();
    delay(10); // delay final por segurança
}
