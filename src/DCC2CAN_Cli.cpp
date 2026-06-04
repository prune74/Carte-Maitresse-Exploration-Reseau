/*
DCC2CAN_Cli.cpp / .h

🎯 Rôle
Interface CLI série du module DCC2CAN.
Elle permet d'afficher des informations de diagnostic simples, comme les
statistiques du décodeur DCC ou le redémarrage de l’ESP32.

📌 Commandes disponibles
- "stats"        → affiche les statistiques DCC (bits, cutout, erreurs)
- "reset"        → redémarre l’ESP32
- "debug on/off" → active/désactive les messages de debug

📌 Particularités
- Le CLI est volontairement minimal : aucune commande liée au Booster,
  à la télémétrie ou au sniffer CAN.
- Le module est non bloquant et s’intègre dans la boucle FreeRTOS.
*/

#include "DCC2CAN_Cli.h"

static String input;

// ---------------------------------------------------------------------------
// Définition d’un handler de commande
// ---------------------------------------------------------------------------
struct CliCommand {
    const char *name;
    void (*handler)(const String &args);
};

// ---------------------------------------------------------------------------
// Handlers des commandes
// ---------------------------------------------------------------------------
void cmd_stats(const String &) {
    uint32_t b0, b1, co, bad;
    DccDecoder_getStats(b0, b1, co, bad);

    Serial.printf(
        "STATS: b0=%lu b1=%lu cutout=%lu bad=%lu\n",
        b0, b1, co, bad
    );
}

void cmd_reset(const String &) {
    Serial.println("Resetting ESP32...");
    delay(100);
    ESP.restart();
}

void cmd_debug_on(const String &) {
    Serial.println("Debug ON");
    // TODO: activer un flag global si nécessaire
}

void cmd_debug_off(const String &) {
    Serial.println("Debug OFF");
    // TODO: désactiver un flag global si nécessaire
}

// ---------------------------------------------------------------------------
// Tableau des commandes
// ---------------------------------------------------------------------------
static const CliCommand commands[] = {
    {"stats",     cmd_stats},
    {"reset",     cmd_reset},
    {"debug on",  cmd_debug_on},
    {"debug off", cmd_debug_off},
};

// ---------------------------------------------------------------------------
// Dispatcher : recherche la commande et appelle son handler
// ---------------------------------------------------------------------------
static bool Cli_dispatch(const String &cmd) {
    for (auto &c : commands) {
        if (cmd.startsWith(c.name)) {
            String args = cmd.substring(strlen(c.name));
            args.trim();
            c.handler(args);
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Initialisation
// ---------------------------------------------------------------------------
void Cli_begin() {
    input.reserve(64);
}

// ---------------------------------------------------------------------------
// Boucle CLI
// ---------------------------------------------------------------------------
void Cli_task() {
    while (Serial.available()) {
        char c = Serial.read();

        if (c == '\n' || c == '\r') {
            if (input.length() == 0)
                return;

            String cmd = input;
            input = "";
            cmd.trim();

            if (!Cli_dispatch(cmd)) {
                Serial.printf("Unknown command: %s\n", cmd.c_str());
            }

            return;
        }

        input += c;
    }
}
