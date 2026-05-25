/*
DiscoveryMaster_Wifi_fl.cpp / .h

🎯 Rôle
Gestion du WiFi pour le module SAMain.
Ce module initialise la connexion réseau en mode AP (point d'accès) ou STA
(client), selon la configuration définie dans DiscoveryMaster_Config.h.

Il fournit une interface simple pour démarrer le WiFi du maître Discovery.

📌 Fonctionnement
- start() :
    • Si WIFI_AP_MODE est défini :
        - démarre un point d'accès WiFi (SSID "digital", password "digital")
        - affiche l'adresse IP du réseau local
    • Sinon (mode STA) :
        - Charge SSID/Password depuis settings.json
        - Se connecte à un réseau WiFi existant
        - Attend la connexion avec timeout de 30s
        - Affiche l'adresse IP obtenue ou échec du timeout

📌 Particularités
- WiFi credentials maintenant chargés depuis SPIFFS (sécurité améliorée)
- Fallback défaut si settings.json absent : "digital"/"digital"
- Mode AP toujours utilise "digital"/"digital" (défaut, peut être changé)
- Section MDNS commentée pour utilisation future
- Mode STA inclut timeout de 30s pour éviter hang infini

🔗 Dépendances
- WiFi (ESP32)
- DiscoveryMaster_Config.h (mode AP/STA)
- DiscoveryMaster_Settings.h (credentials depuis SPIFFS)
*/

#include "DiscoveryMaster_Wifi_fl.h"
#include "DiscoveryMaster_Settings.h"

void DiscoveryMaster_Fl_Wifi::start()
{
#ifdef WIFI_AP_MODE
  // Mode AP - Point d'accès
  WiFi.softAP("digital", "digital");

#ifdef DEBUG
  Serial.print("\n");
  Serial.print("\nWiFi AP Mode - Connected to : ");
  Serial.print("digital");
  Serial.print("\nIP address :   ");
  Serial.print(WiFi.softAPIP());
  Serial.print("\n\n");
#endif

#else
  // Mode STA - Client
  // Load credentials from settings.json
  String ssid = DiscoveryMaster_Settings::getWifiSsid();
  String psw = DiscoveryMaster_Settings::getWifiPassword();

  //  IPAddress local_IP(ip[0], ip[1], ip[2], ip[3]);
  //  IPAddress gateway(ip[0], ip[1], ip[2], 1);
  //  IPAddress subnet(255, 255, 255, 0);

  //  if (!WiFi.config(local_IP, gateway, subnet)) {
  //    Serial.println("STA Failed to configure");
  //  }

  WiFi.begin(ssid.c_str(), psw.c_str());
  
  // Wait for connection with 30 second timeout
  uint32_t timeout = millis() + 30000;
  while (WiFi.status() != WL_CONNECTED && millis() < timeout)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.print("\n");
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print("WiFi STA Mode - Connected to : ");
    Serial.print(ssid);
    Serial.print("\nIP address :   ");
    Serial.print(WiFi.localIP());
    Serial.print("\n\n");
  }
  else
  {
    Serial.print("WiFi STA Mode - Connection FAILED (timeout)\n");
    Serial.print("Fallback to AP mode recommended\n\n");
  }
#endif

  //    if (!MDNS.begin(MDNS_NAME))
  //    {
  //        Serial.print("Error setting up MDNS responder!\n");
  //        while (1)
  //            delay(1000);
  //    }
  //
  ////#ifdef DEBUG
  //    Serial.print("MDNS responder started @ http://");
  //    Serial.print(MDNS_NAME);
  //    Serial.print(".local");
  //    Serial.print("\n\n");
  ////#endif
}
