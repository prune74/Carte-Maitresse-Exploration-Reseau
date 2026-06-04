#pragma once
#include <stdint.h>
#include <initializer_list>

// Pour que CANMessage soit connu (ACAN_ESP32 + ACAN2515)
#include <ACAN_ESP32.h>
#include <ACAN2515.h>

#include "CanID.h"

/**
 * ============================================================================
 *  Classe CanMsg
 * ============================================================================
 *  Représente une trame CAN universelle, indépendante du hardware.
 *
 *  Cette classe encapsule :
 *    - un identifiant CAN (11 bits ou 29 bits)
 *    - un payload de 0 à 8 octets
 *    - une conversion vers/depuis CANMessage (ACAN)
 *
 *  Objectif :
 *    → fournir une abstraction propre, portable, indépendante du driver ACAN
 *    → permettre à l'application de manipuler des messages CAN sans dépendre
 *      de la structure ACAN (CANMessage)
 *
 *  Cette classe ne connaît :
 *    - ni le rôle du bus
 *    - ni le driver utilisé (ESP32 ou MCP2515)
 *    - ni la vitesse
 *    - ni les broches
 *
 *  Elle ne fait qu'une chose :
 *    → représenter un message CAN de manière universelle.
 * ============================================================================
 */
class CanMsg {
public:
    uint32_t id;      // Identifiant CAN (11 ou 29 bits)
    uint8_t  dlc;     // Nombre d'octets utiles (0..8)
    uint8_t  data[8]; // Payload

public:
    // -------------------------------------------------------------------------
    // Constructeurs
    // -------------------------------------------------------------------------

    // Message vide
    CanMsg();

    // ID 11 bits + payload
    CanMsg(uint16_t id11, std::initializer_list<uint8_t> payload);

    // ID 29 bits + payload
    CanMsg(uint32_t id29, std::initializer_list<uint8_t> payload);

    // Conversion ACAN → CanMsg (déclaration seulement)
    CanMsg(const CANMessage &frame);

    // Conversion CanMsg → ACAN (déclaration seulement)
    CANMessage toFrame() const;

    // -------------------------------------------------------------------------
    // Helpers (basés sur CanID)
    // -------------------------------------------------------------------------
    bool is11() const { return CanID::is11(id); }
    bool is29() const { return CanID::is29(id); }

    uint8_t  prio()   const { return CanID::prio(id); }
    uint8_t  cmde()   const { return CanID::cmde(id); }
    uint8_t  resp()   const { return CanID::resp(id); }
    uint16_t nodeId() const { return CanID::nodeId(id); }
};
