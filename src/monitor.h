#ifndef MONITOR_H
#define MONITOR_H

#include "definitions.h"

class ServerController {
private:
    ServerState estado;

    // Token da API
    const String token;
    const String wifi;

public:
    Monitor();

    ServerState getState();
    void updateState();

    void ligar();
    void desligar();

    void (const char *& msg);
};

#endif
