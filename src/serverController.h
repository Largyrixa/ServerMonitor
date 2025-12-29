#ifndef SERVERCONTROLLER_H
#define SERVERCONTROLLER_H

#include "definitions.h"
#include "environment.h"
#include "relayController.h"

#include <Arduino.h>
#include <nvs_flash.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>

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

    const unsigned long pulse_duration_ms;

    WiFiClientSecure client;
    UniversalTelegramBot bot;

    // Controlador do rele
    RelayController *relay;

public:
    // Construtor: inicia os atributos e liga o servidor, se necessário
    ServerController(RelayController *relay, const unsigned long pulse_duration_ms = 1000, const String &botToken = BOT_TOKEN);

    ServerState getState();

    // Atualiza e salva o estado atual na memória
    bool saveState();
    // Carrega o estado atual da memória
    bool loadState();

    // Monitora o servidor e manda logs caso mude o estado
    void watch();

    // Verifica os inputs e trata os comandos
    void handleCommands();

    void powerOn();
    void powerOff();

    // Manda uma mensagem para o chat no telegram
    void sendLog(const String &msg);
};

#endif
