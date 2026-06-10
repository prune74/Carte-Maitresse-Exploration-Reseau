/*
 * DCC2CAN_Cli.cpp
 *
 * 🎯 Rôle
 * Interface CLI série du module DCC2CAN.
 *
 * Cette interface permet d’interagir avec le module via le port série pour :
 *   • afficher les statistiques du décodeur DCC
 *   • ajuster le niveau de debug
 *   • redémarrer l’ESP32
 *
 * Le CLI est volontairement minimal :
 *   • aucune commande liée au Booster
 *   • aucune commande liée au sniffer CAN
 *   • aucune commande bloquante
 *
 * 🛡️ Sécurité temps réel
 * Le CLI ne doit jamais perturber les tâches critiques (DCC, CAN).
 * Les logs détaillés sont protégés via LOG_CRITICAL_DCC.
 */

#include "DCC2CAN_Cli.h"
#include "Debug.h"

// Buffer d’entrée pour la ligne de commande
static String input;

/* ---------------------------------------------------------------------------
 * STRUCTURE D’UNE COMMANDE CLI
 * ------------------------------------------------------------------------- */
struct CliCommand {
    const char *name;
    void (*handler)(const String &args);
};

/* ---------------------------------------------------------------------------
 * HANDLERS DES COMMANDES
 * ------------------------------------------------------------------------- */

// Affiche les statistiques du décodeur DCC
void cmd_stats(const String &)
{
    uint32_t b0, b1, cutout, bad;
    DccDecoder_getStats(b0, b1, cutout, bad);

    LOG_INFO("DCC Stats → b0=%lu b1=%lu cutout=%lu bad=%lu",
             b0, b1, cutout, bad);
}

// Redémarre l’ESP32
void cmd_reset(const String &)
{
    LOG_WARN("Redémarrage ESP32 demandé via CLI");
    delay(100);
    ESP.restart();
}

// Active le mode debug VERBOSE (sécurisé)
void cmd_debug_on(const String &)
{
    // Utilise la fonction sécurisée du module Debug
    DEBUG_LEVEL = DEBUG_VERBOSE;
    LOG_INFO("Debug CLI → mode VERBOSE demandé");
}

// Active uniquement les logs d’erreur
void cmd_debug_off(const String &)
{
    DEBUG_LEVEL = DEBUG_ERROR;
    LOG_INFO("Debug CLI → mode ERROR uniquement");
}

/* ---------------------------------------------------------------------------
 * TABLEAU DES COMMANDES DISPONIBLES
 * ------------------------------------------------------------------------- */
static const CliCommand commands[] = {
    { "stats",     cmd_stats     },
    { "reset",     cmd_reset     },
    { "debug on",  cmd_debug_on  },
    { "debug off", cmd_debug_off },
};

/* ---------------------------------------------------------------------------
 * DISPATCHER : IDENTIFICATION ET EXÉCUTION D’UNE COMMANDE
 * ------------------------------------------------------------------------- */
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

    LOG_WARN("Commande inconnue : %s", cmd.c_str());
    return false;
}

/* ---------------------------------------------------------------------------
 * INITIALISATION DU MODULE CLI
 * ------------------------------------------------------------------------- */
void Cli_begin()
{
    input.reserve(64);
    LOG_INFO("CLI DCC2CAN initialisé");
}

/* ---------------------------------------------------------------------------
 * BOUCLE PRINCIPALE DU CLI
 *
 * À appeler régulièrement dans la boucle FreeRTOS.
 * ------------------------------------------------------------------------- */
void Cli_task()
{
    while (Serial.available())
    {
        char c = Serial.read();

        // Fin de ligne → exécution de la commande
        if (c == '\n' || c == '\r')
        {
            if (input.length() == 0)
                return;

            String cmd = input;
            input = "";
            cmd.trim();

            LOG_CRITICAL_DCC("CLI → commande reçue : %s", cmd.c_str());
            Cli_dispatch(cmd);
            return;
        }

        // Accumulation des caractères
        input += c;
    }
}
