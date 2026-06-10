/*
 * DCC2CAN_CanBooster.cpp
 *
 * Ce module assure la liaison entre le décodeur DCC et le bus CAN Booster.
 * Son rôle est simple mais critique :
 *   - annoncer que le bus CAN0 est prêt
 *   - transmettre chaque bit DCC (bit + phase) sous forme de trame CAN compacte
 *
 * Le Booster utilise ces trames pour reconstruire le signal DCC analogique
 * en temps réel, avec une latence minimale.
 */

#include "DCC2CAN_CanBooster.h"
#include "CanInit.h"
#include "CanMsg.h"
#include "CanBus.h"
#include "Debug.h"

/* ---------------------------------------------------------------------------
   INITIALISATION DU CAN BOOSTER
   ---------------------------------------------------------------------------
   Cette fonction est appelée au démarrage du module DCC2CAN.
   Le contrôleur CAN0 est déjà initialisé par CanInit::begin(), donc ici
   on se contente d’annoncer que le canal est opérationnel.
--------------------------------------------------------------------------- */
void CanBooster_begin()
{
    LOG_INFO("CanBooster → CAN0 prêt (via CanInit)");
}

/* ---------------------------------------------------------------------------
   ENVOI D’UN BIT DCC SUR LE BUS CAN BOOSTER
   ---------------------------------------------------------------------------
   Chaque bit DCC est transmis sous forme d’une trame CAN très compacte :
     - ID  : DCCB_CAN_ID_DCC_BIT (11 bits)
     - DLC : 2 octets
         • data[0] = bit logique (0 ou 1)
         • data[1] = phase (0 = début, 1 = fin)

   Le Booster reconstruit ensuite le signal DCC analogique à partir de ces
   trames, ce qui permet une synchronisation parfaite entre le décodeur
   numérique et la génération du signal physique.
--------------------------------------------------------------------------- */
bool CanBooster_sendDccBit(uint8_t bit, uint8_t phase)
{
    // Construction de la trame CAN (ID 11 bits → cast explicite)
    CanMsg msg((uint16_t)DCCB_CAN_ID_DCC_BIT, { bit, phase });

    // Log détaillé pour le débogage fin
    LOG_VERBOSE("DCC2CAN → bit=%u phase=%u", bit, phase);

    // Envoi sur le bus CAN0 via l’API unifiée CanBus
    bool ok = CanBus::bus(0).send(msg);

    // Avertissement si l’envoi échoue (bus saturé, contrôleur en erreur…)
    if (!ok)
        LOG_WARN("DCC2CAN → Échec d’envoi du bit DCC !");

    return ok;
}
