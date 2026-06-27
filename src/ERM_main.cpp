/*
 * ERM_main.cpp
 *
 * 🎯 Rôle
 * Point d’entrée principal de la Carte Maîtresse d’Exploration du Réseau (ERM).
 *
 * Ce module orchestre :
 *   • l’initialisation du système (WiFi, Web, CAN, paramètres)
 *   • la boucle principale FreeRTOS
 *   • la gestion des Canton Controllers
 *
 * Il constitue le cœur du fonctionnement de la carte.
 */

#include "ERM_main.h"
#include "Variables.h"
#include "ERM_Settings.h"
#include "ERM_CC_Manager.h"
#include "ERM_CanService.h"
#include "ERM_Fl_Wifi.h"
#include "ERM_WebHandler.h"
#include "ERM_Config.h"

#include "CanInit.h"
#include "CAN_Config.h"
#include "Debug.h"

// Configuration CAN globale
extern ERM_Config ERM_CAN_CONFIG;

// Gestion WiFi (local au module)
ERM_Fl_Wifi wifi;

/* ---------------------------------------------------------------------------
 * 🚀 SETUP PRINCIPAL DU MODULE ERM
 * ------------------------------------------------------------------------- */
void ERM_setup()
{
    LOG_INFO("===============================================");
    LOG_INFO("Projet  : %s", PROJECT);
    LOG_INFO("Version : %s", VERSION);
    LOG_INFO("Compilé : %s - %s", __DATE__, __TIME__);
    LOG_INFO("===============================================");

    // LED onboard (optionnel)
    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);

    /* -----------------------------------------------------------
     * PARAMÈTRES
     * --------------------------------------------------------- */
    LOG_INFO("Chargement des paramètres (settings.json)...");
    ERM_Settings::begin();
    ERM_Settings::readFile();

    /* -----------------------------------------------------------
     * INITIALISATION DES BUS CAN
     * --------------------------------------------------------- */
    LOG_INFO("Initialisation des bus CAN (CAN0 + CAN1)...");
    CanInit::begin(ERM_CAN_CONFIG);

    // Laisse le contrôleur TWAI sortir du RESET
    vTaskDelay(pdMS_TO_TICKS(5));

    /* -----------------------------------------------------------
     * WIFI + WEB
     * --------------------------------------------------------- */
    LOG_INFO("Initialisation WiFi...");
    wifi.start();

    LOG_INFO("Initialisation WebHandler (port 80)...");
    webHandler.init(80);

    /* -----------------------------------------------------------
     * SERVICE CAN
     * --------------------------------------------------------- */
    LOG_INFO("Initialisation du service CAN ERM...");
    canService.begin();

    /* -----------------------------------------------------------
     * Canton Controller
     * --------------------------------------------------------- */
    LOG_INFO("Initialisation du gestionnaire de Canton Controllers...");
    CC_Manager.begin();

    LOG_INFO("ERM_setup() terminé");
}

/* ---------------------------------------------------------------------------
 * 🔁 BOUCLE PRINCIPALE DU MODULE ERM
 * ---------------------------------------------------------------------------
 * Exécutée en continu par FreeRTOS.
 * Chaque sous-système dispose de sa propre boucle non bloquante.
 * ------------------------------------------------------------------------- */
void ERM_loop()
{
    canService.loop();
    webHandler.loop();
    CC_Manager.loop();

    // Pause légère pour éviter de saturer le CPU
    vTaskDelay(pdMS_TO_TICKS(10));
}
