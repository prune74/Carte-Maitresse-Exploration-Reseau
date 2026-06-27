#pragma once

#include "CanMsg.h"
#include "CanBus.h"
#include "Variables.h"

/*
 * ERM_CanService.h
 *
 * 🎯 Rôle
 * Service CAN principal de la Carte Maîtresse d’Exploration du Réseau (ERM).
 *
 * Ce module gère :
 *   • la réception et l’analyse des trames CAN
 *   • la supervision du bus (détection silence/erreur)
 *   • l’envoi de commandes vers les Canton Controllers
 *   • les interactions Web → CAN
 *
 * Conception :
 *   • réception non bloquante
 *   • envoi CAN sécurisé
 *   • supervision déterministe
 *   • aucune allocation dynamique
 */

class ERM_CanService
{
public:
    // Constructeur : initialise l’état interne du service
    ERM_CanService();

    // Initialisation du service CAN
    bool begin();

    // Boucle principale de réception CAN
    void loop();

    // Supervision du bus CAN
    bool checkBus(uint32_t timeoutMs = 500);
    uint32_t lastRxAgeMs() const;
    bool isCanOK() const { return _canOK; }

    // Accès à la dernière trame reçue (utilisé par ERS)
    bool getLastFrame(CanMsg &msg);

    // Envoi public d’une trame CAN
    bool sendMessage(const CanMsg &msg);

    // Commandes Canton Controllers
    void ERM_handleCmdTestBus(uint16_t idExp, uint8_t priorite);
    void ERM_handleCmdRequestId(uint16_t idExp, uint8_t priorite);

    // Commandes Web → CAN
    void sendWifiOnOff(bool on);
    void sendExplorationOnOff(bool on);
    void sendSaveAll();
    void sendRestartAll();
    void sendTrackProfile(uint8_t profile);

private:
    // Décodage et traitement d’une trame CAN
    void ERM_handleFrame(const CanMsg &msg);

    // Envoi bas niveau d’une trame CAN
    bool ERM_sendFrame(const CanMsg &msg);

private:
    // Horodatage de la dernière trame reçue
    uint32_t _lastRxTime;

    // Indique si le bus CAN est considéré comme actif
    bool _canOK;

    // Gestion de la dernière trame reçue
    bool hasNewFrame;
    CanMsg lastFrame;
};
