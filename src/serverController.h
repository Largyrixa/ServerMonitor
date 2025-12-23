#ifndef SERVERCONTROLLER_H
#define SERVERCONTROLLER_H

#include "definitions.h"

class ServerController {
private:
    ServerState estado;

    // Token da API
    const String token;
    const String wifi;

public:
    ServerController();

    ServerState getState();
    void updateState();

    void ligar();
    void desligar();

    void SendMsg(const char *& msg);
};

#endif
