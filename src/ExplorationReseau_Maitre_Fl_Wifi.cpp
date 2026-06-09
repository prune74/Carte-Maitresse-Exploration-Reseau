/*
 * ExplorationReseau_Maitre_Fl_Wifi.cpp
 *
 * 🎯 Rôle
 * Gestion du WiFi pour la Carte Maîtresse d’Exploration du Réseau (ERM).
 *
 * Deux modes sont possibles :
 *   • Mode AP  : point d’accès local (SSID = "digital")
 *   • Mode STA : connexion à un réseau existant (SSID/PSW depuis settings.json)
 *
 * Le module est volontairement simple et robuste :
 *   • pas de blocage
 *   • timeout de connexion
 *   • logs détaillés
 */

#include "ExplorationReseau_Maitre_Fl_Wifi.h"
#include "ExplorationReseau_Maitre_Settings.h"
#include "ExplorationReseau_Maitre_Config.h"
#include "Debug.h"
#include <WiFi.h>

// ---------------------------------------------------------------------------
// DÉMARRAGE DU WIFI
// ---------------------------------------------------------------------------
void ERM_Fl_Wifi::start()
{
#ifdef WIFI_AP_MODE
    // -----------------------------------------------------------------------
    // MODE AP (Point d’accès)
    // -----------------------------------------------------------------------
    LOG_INFO("WiFi → Mode AP (Access Point)");

    const char *ssid = "digital";
    const char *psw  = "digital";

    WiFi.softAP(ssid, psw);

    LOG_INFO("WiFi AP → SSID=%s  PSW=%s", ssid, psw);
    LOG_INFO("WiFi AP → IP=%s", WiFi.softAPIP().toString().c_str());

#else
    // -----------------------------------------------------------------------
    // MODE STA (Client)
    // -----------------------------------------------------------------------
    String ssid = ERM_Settings::getWifiSsid();
    String psw  = ERM_Settings::getWifiPassword();

    LOG_INFO("WiFi → Mode STA (Client)");
    LOG_INFO("WiFi STA → Connexion à SSID=\"%s\"", ssid.c_str());

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), psw.c_str());

    // Timeout 30 secondes
    uint32_t timeout = millis() + 30000;
    uint32_t lastDot = 0;

    while (WiFi.status() != WL_CONNECTED && millis() < timeout)
    {
        // Feedback toutes les 500 ms
        if (millis() - lastDot > 500)
        {
            LOG_VERBOSE("WiFi STA → en attente…");
            lastDot = millis();
        }
        vTaskDelay(pdMS_TO_TICKS(50)); // Non bloquant
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        LOG_INFO("WiFi STA → CONNECTÉ à \"%s\"", ssid.c_str());
        LOG_INFO("WiFi STA → IP=%s", WiFi.localIP().toString().c_str());
    }
    else
    {
        LOG_WARN("WiFi STA → ÉCHEC de connexion (timeout)");
        LOG_WARN("WiFi STA → Vérifier SSID/PSW ou activer le mode AP");
    }
#endif
}
