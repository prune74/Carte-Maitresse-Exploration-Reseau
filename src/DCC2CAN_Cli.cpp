/*
DCC2CAN_Cli.cpp / .h

🎯 Rôle
Interface CLI série du module Booster.
Ce module fournit une console texte accessible via le port série, permettant
de diagnostiquer, surveiller et contrôler le fonctionnement du Booster en temps réel.

📌 Fonctionnement
- Analyse les commandes saisies dans le terminal série.
- Associe chaque commande à un handler dédié (tableau de commandes).
- Permet d’afficher les statistiques du décodeur DCC, de redémarrer l’ESP32,
  d’activer le mode debug, ou encore de contrôler le sniffer CAN interne.

📌 Commandes disponibles
- "stats"        → affiche les statistiques DCC (bits, cutout, erreurs)
- "reset"        → redémarre l’ESP32
- "debug on/off" → active/désactive les messages de debug
- "scope on/off" → active le mode oscilloscope (nécessite recompilation)
- "can on/off"   → active/désactive le sniffer CAN interne
- "can filter X" → filtre les trames CAN par ID (hex), ou "off" pour désactiver

📌 Particularités
- Le CLI fonctionne indépendamment du CAN Discovery.
- Le sniffer CAN utilise ACAN_ESP32 et peut afficher les trames reçues en temps réel.
- Le module est léger, non bloquant, et s’intègre parfaitement dans la boucle FreeRTOS
  du Booster (appelé via Booster_loop()).

*/

#include "DCC2CAN_Cli.h"

extern volatile bool canMonitorEnabled;
extern volatile int32_t canMonitorFilter;

static String input;

// ---------------------------------------------------------------------------
// Définition d’un handler de commande
// ---------------------------------------------------------------------------
struct CliCommand
{
    const char *name;
    void (*handler)(const String &args);
};

// ---------------------------------------------------------------------------
// Handlers des commandes
// ---------------------------------------------------------------------------
void cmd_stats(const String &)
{
    uint32_t b0, b1, co, bad;
    DccDecoder_getStats(b0, b1, co, bad);
    Serial.printf("STATS: b0=%lu b1=%lu cutout=%lu bad=%lu\n",
                  b0, b1, co, bad);
}

void cmd_reset(const String &)
{
    Serial.println("Resetting ESP32...");
    delay(100);
    ESP.restart();
}

void cmd_debug_on(const String &)
{
    Serial.println("Debug ON");
}

void cmd_debug_off(const String &)
{
    Serial.println("Debug OFF");
}

void cmd_scope_on(const String &)
{
    Serial.println("Scope mode ON (recompile needed)");
}

void cmd_scope_off(const String &)
{
    Serial.println("Scope mode OFF (recompile needed)");
}

// ---------------- CAN MONITOR (Étape 6) ----------------

void cmd_can_on(const String &)
{
    canMonitorEnabled = true;
    Serial.println("CAN monitor ON");
}

void cmd_can_off(const String &)
{
    canMonitorEnabled = false;
    Serial.println("CAN monitor OFF");
}

void cmd_can_filter(const String &args)
{
    if (args == "off")
    {
        canMonitorFilter = -1;
        Serial.println("CAN filter disabled");
    }
    else
    {
        int id = (int)strtol(args.c_str(), nullptr, 16);
        canMonitorFilter = id;
        Serial.printf("CAN filter set to 0x%03X\n", id);
    }
}

// ---------------------------------------------------------------------------
// Tableau des commandes
// ---------------------------------------------------------------------------
static const CliCommand commands[] = {
    {"stats", cmd_stats},
    {"reset", cmd_reset},
    {"debug on", cmd_debug_on},
    {"debug off", cmd_debug_off},
    {"scope on", cmd_scope_on},
    {"scope off", cmd_scope_off},
    {"can on", cmd_can_on},
    {"can off", cmd_can_off},
    {"can filter", cmd_can_filter},
};

// ---------------------------------------------------------------------------
// Dispatcher : recherche la commande et appelle son handler
// ---------------------------------------------------------------------------
static bool Cli_dispatch(const String &cmd)
{
    for (auto &c : commands)
    {
        if (cmd.startsWith(c.name))
        {
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
void Cli_begin()
{
    input.reserve(64);
}

// ---------------------------------------------------------------------------
// Boucle CLI
// ---------------------------------------------------------------------------
void Cli_task()
{
    while (Serial.available())
    {
        char c = Serial.read();

        if (c == '\n' || c == '\r')
        {
            if (input.length() == 0)
                return;

            String cmd = input;
            input = "";
            cmd.trim();

            if (!Cli_dispatch(cmd))
            {
                Serial.printf("Unknown command: %s\n", cmd.c_str());
            }

            return;
        }

        input += c;
    }
}
