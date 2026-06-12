/*
 * DCC2CAN_CanBooster.cpp
 *
 * Rôle
 * ----
 * Driver d’émission CAN pour la carte maître.
 * Ce module envoie désormais deux types de trames :
 *
 *   1) Trames DCC BIT
 *      → bit logique (0/1) + phase (0/1)
 *
 *   2) Trames CUTOUT
 *      → CUTOUT_START (0)
 *      → CUTOUT_END   (1)
 *
 * Objectif
 * --------
 * Permettre au Booster de reconstruire le signal DCC et de synchroniser
 * parfaitement la fenêtre RailCom sans avoir à deviner ou reconstruire
 * le cutout localement.
 *
 * Contraintes temps réel
 * ----------------------
 * - envoi CAN non bloquant
 * - aucun traitement lourd
 * - logs critiques uniquement en mode test
 */

#include "DCC2CAN_CanBooster.h"
#include "DCC2CAN_Config.h"
#include "CanInit.h"
#include "CanMsg.h"
#include "CanBus.h"
#include "Debug.h"

/* ---------------------------------------------------------------------------
 * INITIALISATION DU DRIVER CAN
 * ------------------------------------------------------------------------- */
void CanBooster_begin()
{
    LOG_INFO("CanBooster → CAN0 initialisé (via CanInit)");
}

/* ---------------------------------------------------------------------------
 * ENVOI D’UN BIT DCC (bit + phase)
 *
 * ID   : DCCB_CAN_ID_DCC_BIT
 * DLC  : 2 octets
 *   data[0] = bit   (0 ou 1)
 *   data[1] = phase (0 ou 1)
 * ------------------------------------------------------------------------- */
bool CanBooster_sendDccBit(uint8_t bit, uint8_t phase)
{
    // Construction de la trame CAN
    CanMsg msg((uint16_t)DCCB_CAN_ID_DCC_BIT, { bit, phase });

    // Log sécurisé (mode test uniquement)
    LOG_CRITICAL_DCC("CAN TX BIT → bit=%u phase=%u", bit, phase);

    // Envoi sur CAN0
    bool ok = CanBus::bus(0).send(msg);

    if (!ok)
        LOG_WARN("CAN TX BIT → échec d’envoi !");

    return ok;
}

/* ---------------------------------------------------------------------------
 * ENVOI D’UN ÉVÉNEMENT CUTOUT
 *
 * ID   : DCCB_CAN_ID_CUTOUT
 * DLC  : 1 octet
 *   data[0] = 0 → CUTOUT_START
 *   data[0] = 1 → CUTOUT_END
 *
 * Cette trame permet au Booster de synchroniser parfaitement la fenêtre
 * RailCom avec le cutout réel généré par la source DCC.
 * ------------------------------------------------------------------------- */
bool CanBooster_sendCutout(uint8_t cutoutType)
{
    // Construction de la trame CAN
    CanMsg msg((uint16_t)DCCB_CAN_ID_CUTOUT, { cutoutType });

    // Log sécurisé (mode test uniquement)
    LOG_CRITICAL_DCC("CAN TX CUTOUT → type=%u", cutoutType);

    // Envoi sur CAN0
    bool ok = CanBus::bus(0).send(msg);

    if (!ok)
        LOG_WARN("CAN TX CUTOUT → échec d’envoi !");

    return ok;
}
