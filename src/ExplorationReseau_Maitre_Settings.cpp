#include "ExplorationReseau_Maitre_Settings.h"
#include "Debug.h"

/*
 * ExplorationReseau_Maitre_Settings.cpp
 *
 * 🎯 Rôle
 * Gestion des paramètres persistants de la Carte Maîtresse ERM.
 *
 * Les paramètres sont stockés dans /settings.json via SPIFFS :
 *   • WiFi (SSID + mot de passe)
 *   • activation exploration
 *   • profil de voie (N / HO)
 *   • mode test (FakeDCC + loopback CAN)
 *
 * Le module fournit :
 *   • begin()      → montage SPIFFS
 *   • readFile()   → chargement JSON
 *   • writeFile()  → sauvegarde JSON
 *   • getters      → valeurs par défaut si vide
 */

// ---------------------------------------------------------------------------
// VARIABLES STATIQUES
// ---------------------------------------------------------------------------
bool     ERM_Settings::WIFI_ON        = true;
bool     ERM_Settings::EXPLORATION_ON = true;
uint16_t ERM_Settings::idNode         = NO_ID;

String   ERM_Settings::WIFI_SSID      = "";
String   ERM_Settings::WIFI_PSW       = "";

uint8_t  ERM_Settings::track_profile  = 0;   // 0 = N (12V), 1 = HO (15V)

// Mode test : FakeDCC + loopback CAN
bool     ERM_Settings::MODE_TEST      = true;

// ---------------------------------------------------------------------------
// INITIALISATION SPIFFS
// ---------------------------------------------------------------------------
void ERM_Settings::begin()
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
void ERM_Settings::readFile()
{
    File file = SPIFFS.open("/settings.json", "r");
    if (!file)
    {
        LOG_WARN("settings.json introuvable → valeurs par défaut utilisées");
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
    idNode         = doc["idNode"]         | NO_ID;
    EXPLORATION_ON = doc["exploration_on"] | true;
    WIFI_ON        = doc["wifi_on"]        | true;

    WIFI_SSID      = doc["wifi_ssid"]      | "";
    WIFI_PSW       = doc["wifi_psw"]       | "";

    track_profile  = doc["track_profile"]  | 0;
    MODE_TEST      = doc["mode_test"]      | true;

    LOG_INFO("Configuration chargée :");
    LOG_INFO(" - idNode         = %u", idNode);
    LOG_INFO(" - exploration_on = %s", EXPLORATION_ON ? "true" : "false");
    LOG_INFO(" - wifi_on        = %s", WIFI_ON ? "true" : "false");
    LOG_INFO(" - wifi_ssid      = %s", WIFI_SSID.c_str());
    LOG_INFO(" - track_profile  = %s",
             track_profile == 0 ? "N (12V)" : "HO (15V)");
    LOG_INFO(" - mode_test      = %s", MODE_TEST ? "true" : "false");

    file.close();
}

// ---------------------------------------------------------------------------
// ÉCRITURE DU FICHIER settings.json
// ---------------------------------------------------------------------------
void ERM_Settings::writeFile()
{
    StaticJsonDocument<512> doc;

    doc["idNode"]         = idNode;
    doc["exploration_on"] = EXPLORATION_ON;
    doc["wifi_on"]        = WIFI_ON;
    doc["wifi_ssid"]      = WIFI_SSID;
    doc["wifi_psw"]       = WIFI_PSW;
    doc["track_profile"]  = track_profile;
    doc["mode_test"]      = MODE_TEST;

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
// Retourne le SSID WiFi, ou une valeur par défaut si vide.
String ERM_Settings::getWifiSsid()
{
    return WIFI_SSID.length() > 0 ? WIFI_SSID : "digital";
}

// Retourne le mot de passe WiFi, ou une valeur par défaut si vide.
String ERM_Settings::getWifiPassword()
{
    return WIFI_PSW.length() > 0 ? WIFI_PSW : "digital";
}
