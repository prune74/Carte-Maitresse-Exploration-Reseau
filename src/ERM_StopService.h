/*
 * ERM_StopService.h
 *
 * 🎯 Rôle
 * Service centralisé de gestion du STOP global pour la Carte Maîtresse ERM.
 *
 * Ce module unifie toute la logique STOP / CLEAR STOP :
 *   • état interne du STOP (bool)
 *   • synchronisation LED STOP
 *   • réception STOP/CLEAR STOP via CAN
 *   • émission STOP/CLEAR STOP (CAN 0x201 / 0x202)
 *   • appels depuis WebHandler, InputService, CanService
 *
 * Avantages :
 *   • aucune duplication de logique
 *   • état cohérent entre CAN, Web, bouton et LED
 *   • CLEAR STOP impossible si STOP non actif
 *   • architecture industrielle, maintenable
 */

#pragma once
#include <stdint.h>

class ERM_StopService
{
public:
    // Initialisation du service (LED STOP)
    static void begin();

    // Actions STOP / CLEAR STOP (émission CAN + mise à jour interne)
    static void triggerStop(); // STOP global (ID 0x201)
    static void clearStop();   // CLEAR STOP (ID 0x202)

    // Appelé par le service CAN lors de la réception d’une trame STOP/CLEAR
    static void onStopReceived();
    static void onClearReceived();

    // État interne du STOP
    static bool isStopActive();

private:
    // État interne du STOP (true = STOP actif)
    static bool _stopActive;
};
