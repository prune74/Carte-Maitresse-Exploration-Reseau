#pragma once
#include "CanMsg.h"

/**
 * ============================================================================
 *  Classe CanBus
 * ============================================================================
 *  Représente UNE interface vers UN bus CAN.
 *
 *  Cette classe est volontairement :
 *    - générique
 *    - indépendante du hardware
 *    - indépendante du rôle du bus (System, Booster, RailCom…)
 *
 *  Elle encapsule un driver ACAN :
 *    - ACAN_ESP32  : contrôleur CAN interne de l’ESP32
 *    - ACAN2515    : contrôleur MCP2515 via SPI
 *
 *  L’application décide :
 *    - combien de bus existent (CAN[0], CAN[1], CAN[2]…)
 *    - leur rôle (System, Booster, Gateway…)
 *    - leur vitesse
 *    - leurs broches
 *
 *  La bibliothèque, elle, ne fait qu’une chose :
 *    → fournir une API unifiée pour envoyer et recevoir des messages CAN.
 *
 *  Exemple d’utilisation (dans l’application) :
 *
 *      extern CanBus CAN[];
 *
 *      // Envoi
 *      CAN[0].send(msg);
 *
 *      // Réception
 *      if (CAN[1].receive(msg)) { ... }
 *
 * ============================================================================
 */
class CanBus {
private:
    /**
     * Pointeur générique vers un driver ACAN.
     *
     * Ce pointeur peut référencer :
     *   - ACAN_ESP32::can
     *   - une instance ACAN2515
     *
     * Le type exact n’a pas d’importance ici :
     * CanBus ne manipule que l’API commune :
     *   - tryToSend()
     *   - receive()
     */
    void *driver;

public:
    // -------------------------------------------------------------------------
    // Constructeur
    // -------------------------------------------------------------------------
    /**
     * @param driverPtr  Pointeur vers un driver ACAN (ESP32 ou MCP2515)
     *
     * Exemple :
     *     CanBus(&ACAN_ESP32::can)
     *     CanBus(&can2515Driver)
     */
    explicit CanBus(void *driverPtr);

    // -------------------------------------------------------------------------
    // Envoi d'un message CAN
    // -------------------------------------------------------------------------
    /**
     * Envoie un message CAN universel (CanMsg).
     * La conversion vers CANMessage (ACAN) est automatique.
     *
     * @return true si l’envoi a réussi
     */
    bool send(const CanMsg &msg);

    // -------------------------------------------------------------------------
    // Réception d'un message CAN
    // -------------------------------------------------------------------------
    /**
     * Tente de recevoir un message CAN.
     * Si une trame est disponible, elle est convertie en CanMsg.
     *
     * @return true si un message a été reçu
     */
    bool receive(CanMsg &msg);
};
