/*
ERM_Config.h

🎯 Rôle
Ce fichier regroupe la configuration locale de la Carte Maîtresse
d’Exploration du Réseau (ERM). Il définit :

  • les constantes système
  • les paramètres généraux du projet
  • les identifiants CAN globaux (format 11 bits)
  • les options de compilation locales

Le protocole complet d’exploration (commandes, trames 29 bits, etc.)
est défini séparément dans Protocol.h.
*/

#pragma once
#include <Arduino.h>
#include <ACAN2515.h>

/* ---------------------------------------------------------------------------
 * Informations projet
 * --------------------------------------------------------------------------- */
#define PROJECT "Carte Maîtresse Exploration Réseau"
#define VERSION "v1.0"

/* ---------------------------------------------------------------------------
 * Debug
 * --------------------------------------------------------------------------- */
#define DEBUG
#define debug Serial

/* ---------------------------------------------------------------------------
 * Constantes système
 * --------------------------------------------------------------------------- */
#define NO_ID 255  // Valeur indiquant l’absence d’identifiant
#define NO_PIN 255 // Valeur indiquant l’absence de broche
#define NB_CC 30   // Nombre maximum de Canton Controllers gérés

/* ---------------------------------------------------------------------------
 * WiFi : Mode Point d’Accès (optionnel)
 * --------------------------------------------------------------------------- */
// #define WIFI_AP_MODE   // Décommenter pour forcer le mode AP
