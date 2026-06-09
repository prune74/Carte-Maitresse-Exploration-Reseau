/*
ExplorationReseau_Maitre_Config.h

🎯 Rôle
Ce fichier regroupe la configuration locale de la Carte Maîtresse
d’Exploration du Réseau (ERM). Il définit :

  • les constantes système
  • les paramètres généraux du projet
  • les identifiants CAN globaux (format 11 bits)
  • les options de compilation locales

Le protocole complet d’exploration (commandes, trames 29 bits, etc.)
est défini séparément dans ExplorationReseau_Protocol.h.
*/

#pragma once
#include <Arduino.h>
#include <ACAN2515.h>

/* ---------------------------------------------------------------------------
 * Informations projet
 * --------------------------------------------------------------------------- */
#define PROJECT "Carte Maitresse Exploration Reseau"
#define VERSION "v1.0"

/* ---------------------------------------------------------------------------
 * Debug
 * --------------------------------------------------------------------------- */
#define DEBUG
#define debug Serial

/* ---------------------------------------------------------------------------
 * Constantes système
 * --------------------------------------------------------------------------- */
#define NO_ID   255     // Valeur indiquant l’absence d’identifiant
#define NO_PIN  255     // Valeur indiquant l’absence de broche
#define NB_SAT  30      // Nombre maximum de satellites gérés

/* ---------------------------------------------------------------------------
 * WiFi : Mode Point d’Accès (optionnel)
 * --------------------------------------------------------------------------- */
//#define WIFI_AP_MODE   // Décommenter pour forcer le mode AP

/* ---------------------------------------------------------------------------
 * 🟥 CAN 11 bits — Messages globaux ERM ↔ Satellites
 * ---------------------------------------------------------------------------
 * Ces identifiants ne suivent pas le format 29 bits du protocole d’exploration.
 * Ils sont utilisés pour la supervision générale du réseau.
 */

/* Heartbeat Satellite → ERM
 * DLC = 2 : [ID_H][ID_L]
 */
#define PROTOCOLCAN_ID_HEARTBEAT      0x200

/* STOP global ERM → Satellites
 * DLC = 0 : arrêt immédiat du réseau
 */
#define PROTOCOLCAN_ID_STOP           0x201

/* CLEAR STOP global ERM → Satellites
 * DLC = 0 : levée du STOP global
 */
#define PROTOCOLCAN_ID_CLEAR_STOP     0x202
