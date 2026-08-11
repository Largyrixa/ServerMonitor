#include "Server.h"
#include "serverController.h"
#include "serverState.h"
#include "environment.h"

using namespace std;

ServerController::ServerController(UniversalTelegramBot &_bot,
                                  const function<bool()> &_onFunc,
                                  const function<bool()> &_offFunc,
                                  const function<ServerState()> &_pingFunc,
                                  const function<String (const String &)> &_commandFunc):
    bot(_bot),
    powerOnFunc(_onFunc),
    powerOffFunc(_offFunc),
    pingFunc(_pingFunc),
    commandFunc(_commandFunc){}


void ServerController::begin() {
    if (!loadState())
        state = ServerState::ERROR;
    
    sendLog("Hello, world!");

    // Re-liga o servidor sozinho após uma queda de energia ou algo assim
    if (state == ServerState::ACTIVE && ping() == ServerState::INACTIVE) {
        // Aguarda 10 segundos para caso o servidor esteja ligando
        delay(10000);
        if (ping() == ServerState::INACTIVE) {
            sendLog("ALERTA: O servidor desligou. Ligando novamente!");
            powerOn();
        }
    }

    const String commands = F(
        "["
        "{\"command\":\"ligar\", \"description\":\"Liga o servidor\"},"
        "{\"command\":\"desligar\", \"description\":\"Desliga o servidor\"},"
        "{\"command\":\"status\", \"description\":\"Verifica o status do servidor\"},"
        "{\"command\":\"do\", \"description\":\"Envia um comando para o servidor\"},"
        "{\"command\":\"help\", \"description\":\"Ajuda na utilização do bot\"},"
        "{\"command\":\"start\", \"description\":\"Mensagem enviada quando você abre o chat com o bot\"}"
        "]"
    );
    bot.setMyCommands(commands);
}

ServerState ServerController::ping() { return pingFunc(); }

ServerState ServerController::getState() { return this->state; }

void ServerController::powerOn()
{
    // Verificação inicial
    state = ping();
    if (state == ServerState::ACTIVE) {
        sendLog("O servidor já está ligado!");
        saveState();
        return;
    }

    String log = "Ligando servidor...";
    sendLog(log);

    // Verificação de erro ao tentar ligar
    if (!powerOnFunc()) {
        state = ServerState::ERROR;
    }
}

void ServerController::powerOff()
{
    // Verificação inicial
    state = ping();
    if (state == ServerState::INACTIVE) {
        sendLog("O servidor já está desligado!");
        saveState();
        return;
    }

    String log = "Desligando servidor...";
    sendLog(log);

    if (!powerOffFunc()) {
        state = ServerState::ERROR;
    }
}

String ServerController::sendCommand(const String &command) { 
    const String response = commandFunc(command);
    state = ping();
    return response;
}

bool ServerController::saveState() {
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
        return true;
}

void ServerController::sendLog(const String &msg) { bot.sendMessage(CHAT_ID, msg, "Markdown"); }

static const String statusMsg(const ServerState &state) {
    switch (state) {
    case ServerState::ACTIVE:
        return "Servidor ativo";
        break;
    case ServerState::BOOTING:
        return "Servidor ligando...";
        break;
    case ServerState::INACTIVE:
        return "Servidor inativo";
        break;
    case ServerState::SHUTTING_DOWN:
        return "Servidor desligando";
        break;
    default:
        return "NO COCONUTS DETECTED!";
        break;
    }
}

void ServerController::loop() {
    auto currentState = ping();

    if (currentState == ServerState::ERROR) {
        sendLog("ATENÇÃO: Servidor com erro!\nVerificação manual necessária");
        return;
    }
    
    // Aviso de mudança de estado
    if (currentState != state) {
        // Dupla verificação para evitar falhas da função de ping
        currentState = ping();
        if (currentState != state) {
            const String msg = statusMsg(currentState);
            sendLog("Aviso de mudança de estado\n" + msg);
        }
        state = currentState;   
    }

    saveState();

    // Tratamento dos comandos do bot
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages) {
        for (int i = 0; i < numNewMessages; i++) {
            telegramMessage &msg = bot.messages[i];
            if (msg.text == "/help" || msg.text == "/start") {
                const String answer = F(
                    "Este bot é um `controlador de servidor`\n"
                    "Se você está usando ele, provavelmente você viu meu [repositório](https://github.com/Largyrixa/ServerController) "
                    "e configurou um monitor para seu servidor.\n"
                    "Se sim, muito obrigado! <3\n\n"
                    "Comandos\n"
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
                const String answer = statusMsg(state);
                sendLog(answer);
            } else if (msg.text.substring(0, 4) == "/do ") {
                const String response = sendCommand(msg.text.substring(4));
                sendLog(response);
            }
        }
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
}
