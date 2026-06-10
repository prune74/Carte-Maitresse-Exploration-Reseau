/*
 * DCC2CAN_CanBooster.cpp
 *
 * 🎯 Rôle
 * Ce module assure la liaison entre le décodeur DCC et le bus CAN Booster.
 * Il transmet chaque bit DCC (bit logique + phase) sous forme d’une trame
 * CAN compacte, permettant au Booster de reconstruire le signal DCC
 * analogique en temps réel avec une latence minimale.
 *
 * Le bus CAN0 est initialisé ailleurs (CanInit::begin()). Ce module se
 * contente donc :
 *   • d’annoncer que le canal est prêt
 *   • d’envoyer les trames CAN correspondant aux bits DCC
 *
 * 🛡️ Sécurité temps réel
 * L’envoi CAN doit être :
 *   • non bloquant
 *   • minimaliste
 *   • sans logs dangereux
 *
 * Les logs détaillés sont protégés via LOG_CRITICAL_DCC :
 *   • actifs uniquement en mode test + DEBUG_VERBOSE
 *   • totalement désactivés en mode réel
 */

#include "DCC2CAN_CanBooster.h"
#include "CanInit.h"
#include "CanMsg.h"
#include "CanBus.h"
#include "Debug.h"

/* ---------------------------------------------------------------------------
 * 🚀 INITIALISATION DU CAN BOOSTER
 *
 * Appelée au démarrage du module DCC2CAN.
 * Le contrôleur CAN0 est déjà configuré par CanInit::begin().
 * ------------------------------------------------------------------------- */
void CanBooster_begin()
{
    LOG_INFO("CanBooster → CAN0 prêt (via CanInit)");
}

/* ---------------------------------------------------------------------------
 * 📡 ENVOI D’UN BIT DCC SUR LE BUS CAN BOOSTER
 *
 * Chaque bit DCC est transmis sous forme d’une trame CAN très compacte :
 *   • ID   : DCCB_CAN_ID_DCC_BIT (11 bits)
 *   • DLC  : 2 octets
 *       - data[0] = bit logique (0 ou 1)
 *       - data[1] = phase (0 = début, 1 = fin)
 *
 * Le Booster reconstruit ensuite le signal DCC analogique à partir de ces
 * trames, assurant une synchronisation parfaite avec le décodeur.
 *
 * ⚠️ Zone critique :
 *   • aucun log non protégé
 *   • aucun traitement lourd
 *   • envoi CAN non bloquant
 * ------------------------------------------------------------------------- */
bool CanBooster_sendDccBit(uint8_t bit, uint8_t phase)
{
    // Construction de la trame CAN (ID 11 bits → cast explicite)
    CanMsg msg((uint16_t)DCCB_CAN_ID_DCC_BIT, { bit, phase });

    // Log sécurisé : actif uniquement en mode test + VERBOSE
    LOG_CRITICAL_DCC("DCC2CAN → bit=%u phase=%u", bit, phase);

    // Envoi sur le bus CAN0 via l’API unifiée CanBus
    bool ok = CanBus::bus(0).send(msg);

    // En cas d’échec, log non critique (rare, non temps réel)
    if (!ok)
        LOG_WARN("DCC2CAN → Échec d’envoi du bit DCC !");

    return ok;
}
