#pragma once
#include <stdint.h>

/*
 * ============================================================================
 *  Protocol.h — Discovery 2026
 * ============================================================================
 *  Structure modernisée :
 *    → une enum class par liaison CAN
 *    → aucune collision d’opcodes
 *    → lisibilité maximale
 *    → documentation naturelle du protocole
 * ============================================================================
 */

/* ============================================================================
 *  ENUM ASPECTS SNCF (EXCC → CC)
 * ==========================================================================*/
enum class ExccAspect : uint8_t
{
    ASPECT_CARRE = 0,
    ASPECT_CARRE_VIOLET,
    ASPECT_SEMAPHORE,
    ASPECT_AVERTISSEMENT,
    ASPECT_RALENTISSEMENT_30,
    ASPECT_RALENTISSEMENT_60,
    ASPECT_RAPPEL_30,
    ASPECT_RAPPEL_60,
    ASPECT_VOIE_LIBRE,
    ASPECT_MANOEUVRE,
    ASPECT_MASQUE,
};

/* ============================================================
 *  🟦 ENUM TYPE DE MATS
 * ============================================================
 */
enum SignalProfil : uint8_t
{
    SIG_ABSENT = 0, // Aucun signal présent physiquement

    SIG_BAL = 1,       // 3 feux : Rouge / Jaune / Vert (BAL)
    SIG_CARRE = 2,     // 5 feux : Carré + Œilleton
    SIG_RAL = 3,       // 7 feux : Ralentissement 30/60
    SIG_RAPPEL = 4,    // 9 feux : Rappel 30/60
    SIG_MANOEUVRE = 5, // 2 feux : Blanc + Violet
};

/* ============================================================================
 *  COMMANDES EXCC → CC (29 bits)
 * ==========================================================================*/
enum class Cmd_EXCC_to_CC : uint16_t
{
    PONG = 0xD1,
    BOOSTER_INFO = 0xD2,
    POSITION_AIGUILLE = 0xD6,
    OCCUPATION = 0xD7,
    PONCTUEL_H = 0xD8,
    PONCTUEL_AH = 0xD9,
    RAILCOM_ADRESSE = 0xDA,
    CALIB_BOOSTER_INFO = 0xDB,
};

/* ============================================================================
 *  COMMANDES CC → EXCC (29 bits)
 * ==========================================================================*/
enum class Cmd_CC_to_EXCC : uint16_t
{
    SERVO_MOVE = 0xF0,
    SERVO_CONFIG = 0xF1,
    SERVO_TEST = 0xF2,
    RECALIBRER_BOOSTER = 0xF3,
    SET_SEUILS = 0xF4,
    BOOSTER_POWER = 0xF5,
    CONFIG_SIGNAUX = 0xF6,
    ASPECT_HORAIRE = 0xF7,
    ASPECT_ANTIHORAIRE = 0xF8,
    DIRECTION_HORAIRE = 0xF9,
    DIRECTION_ANTIHORAIRE = 0xFA,
    OCCUPATION_VOISINS = 0xFB,
    PING = 0xFC,
    CANTON_ID = 0xFD,
    PROFILE_VOIE = 0xFE,
};

/* ============================================================================
 *  COMMANDES CC ↔ CC (Exploitation ferroviaire)
 * ==========================================================================*/
enum class Cmd_CC_to_CC : uint16_t
{
    UPDATE_VOISINS = 0xE0,
    RESERVATION_LOCO = 0xE3,
    RAILCOM_VOISIN = 0xE5,
    ASPECT_VOISIN = 0xE7,
    AIGUILLAGE = 0xE9,
};

/* ============================================================================
 *  COMMANDES CC ↔ CC (Exploration topologique)
 * ==========================================================================*/
enum class Cmd_Exploration_CC : uint16_t
{
    DEMANDE_ID = 0xC0,
    UPDATE_MASQUE_AIG = 0xC1,
    ID_VOISIN = 0xC2,
};

/* ============================================================================
 *  COMMANDES ERM → CC
 * ==========================================================================*/
enum class Cmd_ERM_to_CC : uint16_t
{
    TEST_BUS_REPLY = 0xB3,
    REQUEST_ID_REPLY = 0xB5,
    WIFI_ON_OFF = 0xBD,
    EXPLORATION_ON_OFF = 0xBE,
    SAVE_ALL = 0xBF,
    RESTART_ALL = 0xBC,
    SET_PROFILE = 0x20,
    OFFLINE = 0xC3,
};

/* ============================================================================
 *  COMMANDES CC → ERM
 * ==========================================================================*/
enum class Cmd_CC_to_ERM : uint16_t
{
    TEST_BUS = 0xB2,
    REQUEST_ID = 0xB4,
};

/* ============================================================================
 *  CODES INTERNES EXCC (non-CAN)
 * ==========================================================================*/
enum class ExccCode : uint8_t
{
    POS_DROIT = 0x00,
    POS_DEVIE = 0x01,
    POS_INDET = 0x02,
    POS_INCOHERENT = 0x03,

    PONCT_H_ACTIVE = 0x10,
    PONCT_H_INACTIVE = 0x11,
    PONCT_AH_ACTIVE = 0x12,
    PONCT_AH_INACTIVE = 0x13,

    OCC_ACTIVE = 0x30,
    OCC_LIBRE = 0x31,

    ETAT_OK = 0x00,
    ETAT_BLOQUE = 0x01,
    ETAT_ERREUR = 0x02,
};

/* ============================================================================
 *  COMMANDES GLOBALES 11 bits
 * ==========================================================================*/
enum class Cmd_Global11 : uint16_t
{
    HEARTBEAT = 0x200,
    EMERGENCY_STOP = 0x201,
    CLEAR_STOP = 0x202,
};
