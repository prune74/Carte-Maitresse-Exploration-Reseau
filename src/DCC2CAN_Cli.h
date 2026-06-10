/*
 * DCC2CAN_Cli.h
 *
 * Interface publique du module CLI série du DCC2CAN.
 *
 * Le CLI permet :
 *   - d’afficher les statistiques du décodeur DCC
 *   - de redémarrer l’ESP32
 *   - d’activer/désactiver les logs de debug
 *
 * Le module est non bloquant et s’intègre dans la boucle FreeRTOS.
 */

#pragma once
#include <Arduino.h>
#include "DCC2CAN_DccDecoder.h"
#include <ESP.h>

/* ---------------------------------------------------------------------------
   INITIALISATION DU CLI
   ---------------------------------------------------------------------------
   Prépare le buffer interne et affiche un message d’accueil.
--------------------------------------------------------------------------- */
void Cli_begin();

/* ---------------------------------------------------------------------------
   BOUCLE CLI
   ---------------------------------------------------------------------------
   À appeler régulièrement dans Booster_loop().
   Lit les commandes série et exécute les handlers associés.
--------------------------------------------------------------------------- */
void Cli_task();
