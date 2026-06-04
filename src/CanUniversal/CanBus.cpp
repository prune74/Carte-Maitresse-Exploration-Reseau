#include "CanBus.h"
#include <ACAN_ESP32.h>
#include <ACAN2515.h>

/**
 * ============================================================================
 *  Implémentation de CanBus
 * ============================================================================
 *  Cette classe encapsule un driver ACAN (ESP32 ou MCP2515) derrière une API
 *  unifiée :
 *
 *      - send()    : envoi d'un message CAN
 *      - receive() : réception d'un message CAN
 *
 *  Le code applicatif ne manipule JAMAIS CANMessage (structure ACAN).
 *  Il utilise uniquement CanMsg, qui est indépendant du hardware.
 *
 *  Objectif :
 *    → permettre plusieurs bus CAN simultanés (CAN[0], CAN[1], CAN[2]…)
 *    → sans imposer de rôle (System, Booster, RailCom…)
 *    → sans dépendance au driver dans le reste du code
 *
 * ============================================================================
 */


// -----------------------------------------------------------------------------
// Constructeur
// -----------------------------------------------------------------------------
CanBus::CanBus(void *driverPtr)
    : driver(driverPtr)
{
    // driverPtr peut être :
    //   - &ACAN_ESP32::can
    //   - un pointeur vers une instance ACAN2515
    //
    // Aucun cast ici : on garde le pointeur brut.
}


// -----------------------------------------------------------------------------
// Envoi d'un message CAN
// -----------------------------------------------------------------------------
bool CanBus::send(const CanMsg &msg) {
    // Conversion générique CanMsg → CANMessage (ACAN)
    CANMessage frame = msg.toFrame();

    // -------------------------------------------------------------------------
    // Driver ESP32 interne
    // -------------------------------------------------------------------------
    if (driver == &ACAN_ESP32::can) {
        return ACAN_ESP32::can.tryToSend(frame);
    }

    // -------------------------------------------------------------------------
    // Driver MCP2515 externe
    // -------------------------------------------------------------------------
    ACAN2515 *d = static_cast<ACAN2515*>(driver);
    return d->tryToSend(frame);
}


// -----------------------------------------------------------------------------
// Réception d'un message CAN
// -----------------------------------------------------------------------------
bool CanBus::receive(CanMsg &msg) {
    CANMessage frame;

    // -------------------------------------------------------------------------
    // Driver ESP32 interne
    // -------------------------------------------------------------------------
    if (driver == &ACAN_ESP32::can) {
        if (ACAN_ESP32::can.receive(frame)) {
            msg = CanMsg(frame);   // Conversion CANMessage → CanMsg
            return true;
        }
        return false;
    }

    // -------------------------------------------------------------------------
    // Driver MCP2515 externe
    // -------------------------------------------------------------------------
    ACAN2515 *d = static_cast<ACAN2515*>(driver);
    if (d->receive(frame)) {
        msg = CanMsg(frame);       // Conversion CANMessage → CanMsg
        return true;
    }

    return false;
}
