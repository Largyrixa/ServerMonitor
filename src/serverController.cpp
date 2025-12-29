#include <cstdint>
#include "Server.h"
#include "serverController.h"
#include "definitions.h"
#include "environment.h"
#include "relayController.h"

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
    if ((current_state == ServerState::INACTIVE || current_state == ServerState::ERROR) && (state == ServerState::ACTIVE || state == ServerState::BOOTING)) {
        // Precisamos retomar o estado passado
        powerOn();
    }
}

ServerState ServerController::getState()
{
    const char command[] = "PING";
    Serial.println(command);
    Serial.flush();

    /*
     * Obs: O método Serial.readStringUntil() para de ler após um certo tempo
     * delimitado por Serial.setTimeout(), por isso podemos usá-lo dessa maneira
     */
    const String response = Serial.readStringUntil('\n');
    if (response == "ERRO") {
        return ServerState::ERROR;
    } else if (response == "LIGADO") {
        return ServerState::ACTIVE;
    } else { // Deu timeout
        return ServerState::INACTIVE;
    }
}

void ServerController::powerOn()
{
    // Verificação inicial
    state = getState();
    if (state == ServerState::ACTIVE) {
        sendLog("Servidor já está ligado!");
        return;
    }

    String log = "Ligando servidor...";
    state = ServerState::BOOTING;
    sendLog(log);

    relay->pulse(pulse_duration_ms);

    // Não confiar no relé, apenas no ping
    uint8_t tries = 10;
    while (tries--) {
        state = getState();
        if (state == ServerState::ACTIVE)
            break;
        delay(1000);
    }

    switch (state) {
    case ServerState::ACTIVE:
        log = "Servidor ativo!";
        sendLog(log);
        break;

    case ServerState::INACTIVE:
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
    state = ServerState::SHUTTING_DOWN;
    sendLog(log);

    // Manda o comando de desligar para o servidor
    const char command[] = "DESLIGAR";
    Serial.println(command);
    delay(500);

    // Lê o serial até o caractere de nova linha
    String response = Serial.readStringUntil('\n');
    if (response == "ERRO") {
        state = ServerState::ERROR;
    } else {
        state = getState();
        uint8_t tries = 5;
        while (tries--) {
            if (state == ServerState::INACTIVE)
                break;
            delay(1000);
            state = getState();
        }
        sendLog(log);
    }

    switch (state) {
    case ServerState::ACTIVE:
        log = "Não foi possível desligar o servidor!";
        sendLog(log);
        break;

    case ServerState::INACTIVE:
        log = "Servidor inativo!";
        sendLog(log);
        break;

    default:
        log = "Erro ao tentar desligar o servidor!";
        sendLog(log);
        break;
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

    const char *chave_nvs = "ESTADO SERVIDOR";

    uint8_t tmp = static_cast<uint8_t>(state);
    err = nvs_set_u8(handler, chave_nvs, tmp);

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

    const char *chave_nvs = "ESTADO SERVIDOR";
    uint8_t tmp;
    err = nvs_get_u8(handler, chave_nvs, &tmp);

    if (tmp < static_cast<uint8_t>(ServerState::_COUNT))
        state = static_cast<ServerState>(tmp);

    nvs_close(handler);
    if (err != ESP_OK)
        return false;
    else
        return false;
}

void ServerController::sendLog(const String &msg) { bot.sendMessage(CHAT_ID, msg); }
