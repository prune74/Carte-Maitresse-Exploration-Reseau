#include "DiscoveryMaster_Settings.h"
#include "Debug.h"

// ---------------------------------------------------------------------------
// VARIABLES STATIQUES
// ---------------------------------------------------------------------------
bool     DiscoveryMaster_Settings::WIFI_ON        = true;
bool     DiscoveryMaster_Settings::DISCOVERY_ON   = true;
uint16_t DiscoveryMaster_Settings::idNode         = NO_ID;
String   DiscoveryMaster_Settings::WIFI_SSID      = "";
String   DiscoveryMaster_Settings::WIFI_PSW       = "";
uint8_t  DiscoveryMaster_Settings::track_profile  = 0;   // 0 = N, 1 = HO

// ---------------------------------------------------------------------------
// INITIALISATION SPIFFS
// ---------------------------------------------------------------------------
void DiscoveryMaster_Settings::begin()
{
    if (!SPIFFS.begin(true))
    {
        LOG_ERROR("SPIFFS → échec du montage");
        return;
    }

    LOG_INFO("SPIFFS monté avec succès");
}

// ---------------------------------------------------------------------------
// LECTURE DU FICHIER settings.json
// ---------------------------------------------------------------------------
void DiscoveryMaster_Settings::readFile()
{
    File file = SPIFFS.open("/settings.json", "r");
    if (!file)
    {
        LOG_WARN("settings.json introuvable → utilisation des valeurs par défaut");
        return;
    }

    StaticJsonDocument<768> doc;
    DeserializationError error = deserializeJson(doc, file);

    if (error)
    {
        LOG_ERROR("Erreur JSON dans settings.json → valeurs par défaut");
        file.close();
        return;
    }

    // Lecture sécurisée des paramètres
    idNode        = doc["idNode"]        | NO_ID;
    DISCOVERY_ON  = doc["discovery_on"]  | true;
    WIFI_ON       = doc["wifi_on"]       | true;

    WIFI_SSID     = doc["wifi_ssid"]     | "";
    WIFI_PSW      = doc["wifi_psw"]      | "";

    track_profile = doc["track_profile"] | 0;

    LOG_INFO("Configuration chargée :");
    LOG_INFO(" - idNode        = %u", idNode);
    LOG_INFO(" - discovery_on  = %s", DISCOVERY_ON ? "true" : "false");
    LOG_INFO(" - wifi_on       = %s", WIFI_ON ? "true" : "false");
    LOG_INFO(" - wifi_ssid     = %s", WIFI_SSID.c_str());
    LOG_INFO(" - track_profile = %s",
             track_profile == 0 ? "N (12V)" : "HO (15V)");

    file.close();
}

// ---------------------------------------------------------------------------
// ÉCRITURE DU FICHIER settings.json
// ---------------------------------------------------------------------------
void DiscoveryMaster_Settings::writeFile()
{
    StaticJsonDocument<512> doc;

    doc["idNode"]        = idNode;
    doc["discovery_on"]  = DISCOVERY_ON;
    doc["wifi_on"]       = WIFI_ON;
    doc["wifi_ssid"]     = WIFI_SSID;
    doc["wifi_psw"]      = WIFI_PSW;
    doc["track_profile"] = track_profile;

    File file = SPIFFS.open("/settings.json", "w");
    if (!file)
    {
        LOG_ERROR("SPIFFS → impossible d’ouvrir settings.json en écriture");
        return;
    }

    serializeJsonPretty(doc, file);
    file.close();

    LOG_INFO("settings.json mis à jour");
}

// ---------------------------------------------------------------------------
// GETTERS
// ---------------------------------------------------------------------------
String DiscoveryMaster_Settings::getWifiSsid()
{
    return WIFI_SSID.length() > 0 ? WIFI_SSID : "digital";
}

String DiscoveryMaster_Settings::getWifiPassword()
{
    return WIFI_PSW.length() > 0 ? WIFI_PSW : "digital";
}
