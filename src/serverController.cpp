#include "serverController.h"
#include "definitions.h"
#include "environment.h"
#include "relayController.h"
#include <cstdint>

/*
ServerConfig::ServerConfig()
{
    serverIP.fromString(SERVER_IP);
    botToken = BOT_TOKEN;
    serverPort = SERVER_PORT;
}
*/

ServerController::ServerController(RelayController *relay,  const unsigned long pulse_duration_ms, const String &botToken) :
relay(relay), pulse_duration_ms(pulse_duration_ms), bot(botToken, client)
{
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
    loadState();
    ServerState current_state = getState();
    if ((current_state == ServerState::DESLIGADO || current_state == ServerState::ERRO) &&
        (state == ServerState::LIGADO || state == ServerState::LIGANDO)) { // Precisamos retomar o estado passado
            powerOn();
    }
}

ServerState ServerController::getState()
{
    const char command[] = "PING";
    Serial.println(command);
    delay(500);

    /*
     * Obs: O método Serial.readStringUntil() para de ler após um certo tempo
     * delimitado por Serial.setTimeout(), por isso podemos usá-lo dessa maneira
     */
    const String response = Serial.readStringUntil('\n');
    if (response == "ERRO") {
        return ServerState::ERRO;
    } else if (response == "LIGADO") {
        return ServerState::LIGADO;
    } else { // Deu timeout
        return ServerState::DESLIGADO;
    }
}

void ServerController::powerOn()
{
    // Verificação inicial
    state = getState();
    if (state == ServerState::LIGADO) {
        sendLog("Servidor já está ligado!");
        return;
    }

    String log = "Ligando servidor...";
    state = ServerState::LIGANDO;
    sendLog(log);

    relay->pulse(pulse_duration_ms);

    // Não confiar no relé, apenas no ping
    uint8_t tries = 10;
    while (tries--) {
        state = getState();
        delay(1000);
    }

    state = getState();
    switch (state) {
    case ServerState::LIGADO:
        log = "Servidor ativo!";
        sendLog(log);
        break;

    case ServerState::DESLIGADO:
        log = "Não foi possível ligar o servidor!";
        sendLog(log);
        break;

    default:
        log = "Erro ao tentar ligar o servidor!";
        sendLog(log);
        break;
    }
    saveState();
}

void ServerController::powerOff()
{
    String log = "Desligando servidor...";
    state = ServerState::DESLIGANDO;
    sendLog(log);

    // Manda o comando de desligar para o servidor
    const char command[] = "DESLIGAR";
    Serial.println(command);
    delay(500);

    // Lê o serial até o caractere de nova linha
    String response = Serial.readStringUntil('\n');
    if (response == "ERRO") {
        log = "Erro ao tentar desligar o servidor!";
        sendLog(log);
    } else {
        state = getState();
        uint8_t tries = 10;
        while (tries--) {
            if (state == ServerState::DESLIGADO) {
                log = "Server inativo!";
                sendLog(log);
            }
            delay(1000);
            state = getState();
        }
        log = "Erro ao tentar desligar o servidor!";
        sendLog(log);
    }
    saveState();
}

bool ServerController::saveState()
{
    state = getState();

    nvs_handle handler;
    esp_err_t err;

    err = nvs_flash_init_partition("nvs");

    if (err != ESP_OK)
        return false;

    err = nvs_open_from_partition("nvs", "ns_nvs", NVS_READWRITE, &handler);

    if (err != ESP_OK)
        return false;

    const String chave_nvs = "ESTADO SERVIDOR";

    err = nvs_set_u8(handler, chave_nvs, (uint8_t)state);

    if (err != ESP_OK) {
        nvs_close(handler);
        return false;
    } else {
        nvs_commit(handler);
        nvs_close(handler);
        return true;
    }
}

bool ServerController::loadState()
{
    nvs_handle handler;
    esp_err_t err;

    err = nvs_flash_init_partition("nvs");
    if (err != ESP_OK)
        return false;

    err = nvs_open_from_partition("nvs", "ns_nvs", NVS_READONLY, &handler);

    if (err != ESP_OK)
        return false;

    const String chave_nvs = "ESTADO SERVIDOR";
    err = nvs_get_u8(handler, chave_nvs, &state);

    nvs_close(handler);
    if (err != ESP_OK)
        return false;
    else
        return false;
}

void ServerController::sendLog(const String &msg) { bot.sendMessage(CHAT_ID, msg); }
