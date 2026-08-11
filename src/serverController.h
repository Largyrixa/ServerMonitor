#include "esp_wifi_types_generic.h"
#ifndef SERVERCONTROLLER_H
#define SERVERCONTROLLER_H

#include "serverState.h"
#include "environment.h"

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
    const std::function<bool()> powerOnFunc;
    const std::function<bool()> powerOffFunc;
    const std::function<ServerState()> pingFunc;
    const std::function<String (const String &)> commandFunc;

public:
    // Construtor: inicia os atributos
    ServerController(UniversalTelegramBot &_bot,
                     const std::function<bool()> &_onFunc,
                     const std::function<bool()> &_offFunc,
                     const std::function<ServerState()> &_pingFunc,
                     const std::function<String(const String &)> &_commandFunc);

    // Inicializa o controlador, liga o servidor, se necessário
    void begin();

    // Retorna o estado atual do servidor, independente do que está salvo (não salva o valor)
    ServerState ping();

    // Retorna o valor salvo do estado do servidor
    ServerState getState();

    // Atualiza e salva o estado atual na memória
    bool saveState();
    
    // Carrega o estado atual da memória
    bool loadState();

    // Monitora o servidor e manda logs caso mude o estado
    void loop();

    // Manda um comando pelo serial do servidor
    String sendCommand(const String &command);

    void powerOn();
    void powerOff();

    // Manda uma mensagem para o chat no telegram
    void sendLog(const String &msg);
};

#endif
