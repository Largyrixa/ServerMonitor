#include "esp_wifi_types_generic.h"
#ifndef SERVERCONTROLLER_H
#define SERVERCONTROLLER_H

#include "serverState.h"
#include "environment.h"
#include "relayController.h"

#include <Arduino.h>
#include <nvs_flash.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <functional>

/*
struct ServerConfig {
    IPAddress serverIP;
    String botToken;
    uint16_t serverPort;

    // Construtor com defaults do environment.h
    ServerConfig();
};
*/

class ServerController {
private:
    ServerState state;
    UniversalTelegramBot &bot;

    // funções para ligar e desligar
    std::function<bool()> powerOnFunc;
    std::function<bool()> powerOffFunc;
    std::function<ServerState()> pingFunc;

public:
    // Construtor: inicia os atributos
    ServerController(UniversalTelegramBot &_bot,
                     std::function<bool()> _onFunc,
                     std::function<bool()> _offFunc,
                     std::function<ServerState()> _pingFunc);

    // Inicializa o controlador, liga o servidor, se necessário
    void begin();

    ServerState getState();

    // Atualiza e salva o estado atual na memória
    bool saveState();
    
    // Carrega o estado atual da memória
    bool loadState();

    // Monitora o servidor e manda logs caso mude o estado
    void loop();

    // Verifica os inputs e trata os comandos
    void handleCommands();

    void powerOn();
    void powerOff();

    // Manda uma mensagem para o chat no telegram
    void sendLog(const String &msg);
};

#endif
