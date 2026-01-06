#include "Server.h"
#include "serverController.h"
#include "serverState.h"
#include "environment.h"
#include "relayController.h"

using namespace std;

ServerController::ServerController(UniversalTelegramBot &_bot,
                                  function<bool()> _onFunc,
                                  function<bool()> _offFunc,
                                  function<ServerState()> _pingFunc):
    bot(_bot),
    powerOnFunc(_onFunc),
    powerOffFunc(_offFunc),
    pingFunc(_pingFunc){}

void ServerController::begin() {
    if (!loadState())
        state = ServerState::ERROR;
    
    const ServerState currentState = getState();
    if ((currentState == ServerState::INACTIVE || currentState == ServerState::ERROR) && (state == ServerState::ACTIVE || state == ServerState::BOOTING)) {
        sendLog("ALERTA: O servidor desligou. Ligando novamente!");
        powerOn();
    }

    const String commands = F(
        "["
        "{\"command\":\"start\", \"description\":\"Mensagem enviada quando você abre o chat com o bot\"},"
        "{\"command\":\"help\", \"description\":\"Ajuda na utilização do bot\"},"
        "{\"command\":\"ligar\", \"description\":\"Liga o servidor\"},"
        "{\"command\":\"desligar\", \"description\":\"Desliga o servidor\"},"
        "{\"command\":\"status\", \"description\":\"Verifica o status do servidor\"},"
        "{\"command\":\"do\", \"description\":\"NÃO IMPLEMENTADO AINDA\"}"
        "]"
    );
    bot.setMyCommands(commands);
}

ServerState ServerController::getState() { return pingFunc(); }

void ServerController::powerOn()
{
    // Verificação inicial
    state = getState();
    if (state == ServerState::ACTIVE) {
        sendLog("Servidor já está ligado!");
        saveState();
        return;
    }

    String log = "Ligando servidor...";
    state = ServerState::BOOTING;
    sendLog(log);

    for (int i = 0; i < 5 && state != ServerState::ACTIVE; i++)  {
        // Verificação de erro ao tentar ligar
        if (!powerOnFunc()) {
            state = ServerState::ERROR;
            break;
        }

        // Espera 10 segundos para o servidor ligar
        delay(10000);
        state = getState();
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
    // Verificação inicial
    state = getState();
    if (state == ServerState::INACTIVE) {
        sendLog("O servidor já está desligado!");
        saveState();
        return;
    }

    String log = "Desligando servidor...";
    state = ServerState::SHUTTING_DOWN;
    sendLog(log);

    for (int i = 0; i < 5 && state != ServerState::INACTIVE; i++) {
        if (!powerOffFunc()) {
            state = ServerState::ERROR;
            break;
        }

        // Espera 10 segundos para o servidor desligar
        delay(10000);
        state = getState();
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

String ServerController::sendCommand(const String &command) {
    if (state != ServerState::ACTIVE) {
        return "ERRO: Servidor não está ativo!";
    }

    Serial.println("/do " + command);
    Serial.flush();
    
    String resposta = "";
    while (true) {
        const String parte = Serial.readStringUntil('\n');
        if (parte == "/fim") {
            return resposta;
        } else {
            resposta += parte;
        }
    }
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

void ServerController::sendLog(const String &msg) { bot.sendMessage(CHAT_ID, msg, "Markdown"); }

void ServerController::tick() {
    const auto currentState = getState();

    if (currentState == ServerState::ERROR) {
        sendLog("ATENÇÃO: Servidor com erro!\nVerificação manual necessária")
    }

    // Se o servidor desligou e estava ligado
    if ((currentState == ServerState::INACTIVE || currentState == ServerState::SHUTTING_DOWN) &&
        (state == ServerState::ACTIVE || state == ServerState::BOOTING)) {
        powerOn();
    }
    saveState();

    // Tratamento dos comandos do bot
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
        for (int i = 0; i < numNewMessages; i++) {
            telegramMessage &msg = bot.messages[i];
            if (msg.text == "/help" || msg.text == "/start") {
                const String answer = F(
                    "# Este bot é um `controlador de servidor`\n",
                    "Se você está usando ele, provavelmente você viu meu [repositório](https://github.com/Largyrixa/ServerController)"
                    "e configurou um monitor para seu servidor.\n"
                    "Se sim, muito obrigado! <3\n"
                    "# Comandos\n"
                    "- `/ligar`: liga o servidor\n"
                    "- `/desligar`: desliga o servidor\n"
                    "- `/status`: retorna o estado do servidor\n"
                    "- `/do comando`: envia um comando para o terminal do servidor"
                );
                sendLog(answer);
            } else if (msg.text == "/ligar") {
                powerOn();
            } else if (msg.text == "/desligar") {
                powerOff();
            } else if (msg.text == "/status") {
                String answer;
                switch (state) {
                    case ServerState::ACTIVE:
                        answer = "Servidor ativo 👍🥥";
                        break;
                    case ServerState::BOOTING:
                        answer = "Guenta aí, servidor ligando ✋🥥";
                        break;
                    case ServerState::INACTIVE:
                        answer = "Servidor inativo 🥥👎";
                        break;
                    case ServerState::SHUTTING_DOWN:
                        answer = "Servidor desligando ❌🥥";
                        break;
                    default:
                        answer = "NO COCONUTS DETECTED!";
                        break;
                }
                sendLog(answer);
            } else if (msg.text.substring(0, 3) == "/do ") {
                const String response = sendCommand(msg.text.substring(4));
                sendLog(response);
            }
        }
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
}
