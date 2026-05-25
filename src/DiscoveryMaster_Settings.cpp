/*
DiscoveryMaster_Settings.cpp / .h

🎯 Rôle
Gestion centralisée des paramètres persistants du module DiscoveryMaster.
Ce module lit et écrit le fichier JSON de configuration (settings.json) stocké
dans SPIFFS, et fournit des variables statiques accessibles à tout le système.

📌 Fonctionnement
- begin() :
    • Monte le système de fichiers SPIFFS
    • Affiche un message d'erreur si le montage échoue

- readFile() :
    • Ouvre settings.json en lecture
    • Charge les paramètres dans un document JSON
    • Récupère :
        - idNode        → dernier ID attribué à un satellite
        - discovery_on  → activation du réseau Discovery
        - wifi_on       → activation du WiFi
        - wifi_ssid     → SSID pour le mode WiFi STA
        - wifi_psw      → Mot de passe WiFi STA
    • Utilise des valeurs par défaut si le fichier est absent ou invalide

- writeFile() :
    • Construit un document JSON avec les paramètres actuels
    • Écrit settings.json dans SPIFFS
    • Sauvegarde persistante après modification

📌 Particularités
- Les variables sont statiques, donc accessibles sans instance
- Les identifiants WiFi sont désormais stockés dans settings.json (pas en dur)
- Permet de changer SSID/PSW sans recompiler le firmware

🔗 Dépendances
- SPIFFS (stockage flash)
- ArduinoJson (sérialisation/désérialisation)
- DiscoveryMaster_Config.h (NO_ID, constantes système)
*/

#include "DiscoveryMaster_Settings.h"

bool DiscoveryMaster_Settings::WIFI_ON = true;
bool DiscoveryMaster_Settings::DISCOVERY_ON = true;
uint16_t DiscoveryMaster_Settings::idNode = NO_ID;
String DiscoveryMaster_Settings::WIFI_SSID = "";
String DiscoveryMaster_Settings::WIFI_PSW = "";

void DiscoveryMaster_Settings::begin()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("Erreur : impossible de monter SPIFFS");
        return;
    }

    Serial.println("SPIFFS monté avec succès");
}

void DiscoveryMaster_Settings::readFile()
{
    File file = SPIFFS.open("/settings.json", "r");
    if (!file)
    {
        Serial.println("settings.json introuvable → valeurs par défaut");
        return;
    }

    StaticJsonDocument<768> doc;
    DeserializationError error = deserializeJson(doc, file);

    if (error)
    {
        Serial.println("Erreur JSON → valeurs par défaut");
        file.close();
        return;
    }

    // Lecture sécurisée des paramètres
    DiscoveryMaster_Settings::idNode = doc["idNode"] | NO_ID;
    DiscoveryMaster_Settings::DISCOVERY_ON = doc["discovery_on"] | true;
    DiscoveryMaster_Settings::WIFI_ON = doc["wifi_on"] | true;
    
    // Credentials WiFi
    DiscoveryMaster_Settings::WIFI_SSID = doc["wifi_ssid"] | "";
    DiscoveryMaster_Settings::WIFI_PSW = doc["wifi_psw"] | "";

    Serial.printf("ID dernier satellite : %d\n", DiscoveryMaster_Settings::idNode);
    Serial.printf("WiFi SSID : %s\n", DiscoveryMaster_Settings::WIFI_SSID.c_str());

    file.close();
}

void DiscoveryMaster_Settings::writeFile()
{
    StaticJsonDocument<512> doc;

    doc["idNode"] = DiscoveryMaster_Settings::idNode;
    doc["discovery_on"] = DiscoveryMaster_Settings::DISCOVERY_ON;
    doc["wifi_on"] = DiscoveryMaster_Settings::WIFI_ON;
    doc["wifi_ssid"] = DiscoveryMaster_Settings::WIFI_SSID;
    doc["wifi_psw"] = DiscoveryMaster_Settings::WIFI_PSW;

    File file = SPIFFS.open("/settings.json", "w");
    if (!file)
    {
        Serial.println("Erreur : impossible d'ouvrir settings.json en écriture");
        return;
    }

    serializeJsonPretty(doc, file);
    file.close();

    Serial.println("settings.json mis à jour");
}

String DiscoveryMaster_Settings::getWifiSsid()
{
    return WIFI_SSID.length() > 0 ? WIFI_SSID : "digital";
}

String DiscoveryMaster_Settings::getWifiPassword()
{
    return WIFI_PSW.length() > 0 ? WIFI_PSW : "digital";
}
