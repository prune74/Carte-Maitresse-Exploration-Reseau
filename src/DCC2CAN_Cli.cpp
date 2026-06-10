/*
 * DCC2CAN_Cli.cpp
 *
 * Interface CLI série du module DCC2CAN.
 *
 * Cette interface permet d’interagir directement avec le module via le port
 * série. Elle est conçue pour fournir un diagnostic rapide du décodeur DCC
 * et pour exécuter quelques commandes de maintenance.
 *
 * Le CLI est volontairement minimal :
 *   - aucune commande liée au Booster
 *   - aucune commande liée au sniffer CAN
 *   - aucune commande bloquante
 *
 * Le module s’intègre naturellement dans la boucle FreeRTOS et ne perturbe
 * jamais les tâches temps réel du décodeur.
 */

#include "DCC2CAN_Cli.h"
#include "Debug.h"

// Buffer d’entrée pour la ligne de commande
static String input;

/* ---------------------------------------------------------------------------
   STRUCTURE D’UNE COMMANDE CLI
   ---------------------------------------------------------------------------
   Chaque commande est définie par :
     - un nom (chaîne)
     - un handler (fonction appelée lorsque la commande est reconnue)
--------------------------------------------------------------------------- */
struct CliCommand {
    const char *name;
    void (*handler)(const String &args);
};

/* ---------------------------------------------------------------------------
   HANDLERS DES COMMANDES
   ---------------------------------------------------------------------------
   Chaque handler reçoit la chaîne d’arguments (souvent vide) et exécute
   l’action correspondante.
--------------------------------------------------------------------------- */

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

// Active le mode debug VERBOSE
void cmd_debug_on(const String &)
{
    DEBUG_LEVEL = DEBUG_VERBOSE;
    LOG_INFO("Debug CLI → mode VERBOSE activé");
}

// Active uniquement les logs d’erreur
void cmd_debug_off(const String &)
{
    DEBUG_LEVEL = DEBUG_ERROR;
    LOG_INFO("Debug CLI → mode ERROR uniquement");
}

/* ---------------------------------------------------------------------------
   TABLEAU DES COMMANDES DISPONIBLES
   ---------------------------------------------------------------------------
   Le dispatcher parcourt ce tableau pour identifier la commande reçue.
--------------------------------------------------------------------------- */
static const CliCommand commands[] = {
    { "stats",     cmd_stats     },
    { "reset",     cmd_reset     },
    { "debug on",  cmd_debug_on  },
    { "debug off", cmd_debug_off },
};

/* ---------------------------------------------------------------------------
   DISPATCHER : IDENTIFICATION ET EXÉCUTION D’UNE COMMANDE
   ---------------------------------------------------------------------------
   Le dispatcher compare la chaîne reçue avec chaque entrée du tableau.
   Si une correspondance est trouvée, le handler associé est exécuté.
--------------------------------------------------------------------------- */
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
   INITIALISATION DU MODULE CLI
   ---------------------------------------------------------------------------
   Réservations mémoire et message d’accueil.
--------------------------------------------------------------------------- */
void Cli_begin()
{
    input.reserve(64);
    LOG_INFO("CLI DCC2CAN initialisé");
}

/* ---------------------------------------------------------------------------
   BOUCLE PRINCIPALE DU CLI
   ---------------------------------------------------------------------------
   Cette fonction doit être appelée régulièrement dans la boucle FreeRTOS.
   Elle lit les caractères reçus sur le port série et déclenche l’exécution
   d’une commande lorsqu’une ligne complète est détectée.
--------------------------------------------------------------------------- */
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

            LOG_VERBOSE("CLI → commande reçue : %s", cmd.c_str());
            Cli_dispatch(cmd);
            return;
        }

        // Accumulation des caractères
        input += c;
    }
}
