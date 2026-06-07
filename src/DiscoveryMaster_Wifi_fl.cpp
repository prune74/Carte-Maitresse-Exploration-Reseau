#include "DiscoveryMaster_Wifi_fl.h"
#include "DiscoveryMaster_Settings.h"
#include "DiscoveryMaster_Config.h"
#include "Debug.h"
#include <WiFi.h>

void DiscoveryMaster_Fl_Wifi::start()
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
    String ssid = DiscoveryMaster_Settings::getWifiSsid();
    String psw  = DiscoveryMaster_Settings::getWifiPassword();

    LOG_INFO("WiFi → Mode STA (Client)");
    LOG_INFO("WiFi STA → Connexion à SSID=\"%s\"", ssid.c_str());

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), psw.c_str());

    // Timeout 30 secondes
    uint32_t timeout = millis() + 30000;
    uint32_t lastDot = 0;

    while (WiFi.status() != WL_CONNECTED && millis() < timeout)
    {
        // Petit feedback toutes les 500 ms
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
        LOG_WARN("WiFi STA → Vérifier SSID/PSW ou passer en mode AP");
    }
#endif
}
