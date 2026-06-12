/*
 * DCC2CAN_TaskCan.cpp
 *
 * Rôle
 * ----
 * Tâche FreeRTOS responsable de l’émission périodique des trames CAN
 * contenant le bit DCC courant (bit logique + phase).
 *
 * Cette tâche constitue le lien entre :
 *   • l’état logique du flux DCC (BoosterState)
 *   • le bus CAN Booster (via CanBooster_sendDccBit)
 *
 * Fonctionnement
 * --------------
 *   - toutes les 2 ms, la tâche demande à BoosterState d’envoyer
 *     le bit DCC courant
 *   - BoosterState_sendCan() applique sa logique interne :
 *        • anti-spam (envoi uniquement si changement)
 *        • respect de l’état RUNNING / STOP
 *
 * Contraintes temps réel
 * ----------------------
 * Cette tâche n’est PAS critique (contrairement à taskDcc).
 * Elle tourne à faible priorité et ne doit jamais bloquer la chaîne
 * DCC → état → CAN.
 *
 * Les logs sont protégés via LOG_CRITICAL_DCC :
 *   • actifs uniquement en mode test + DEBUG_VERBOSE
 *   • totalement désactivés en mode réel
 */

#include "DCC2CAN_TaskCan.h"
#include "DCC2CAN_State.h"
#include "Debug.h"

/* ---------------------------------------------------------------------------
 * TÂCHE CAN (TX)
 *
 * Cadence : 2 ms
 * Priorité : faible
 *
 * Cette tâche :
 *   • appelle BoosterState_sendCan() pour émettre le bit courant
 *   • ne traite PAS les événements CUTOUT (envoyés immédiatement ailleurs)
 *   • ne fait aucun traitement lourd
 * ------------------------------------------------------------------------- */
void taskCan(void *pv)
{
    (void)pv;

    LOG_INFO("Tâche CAN (TX) démarrée → cadence 2 ms");

    for (;;)
    {
        /* -----------------------------------------------------------
         * Demande à BoosterState d’émettre le bit DCC courant.
         * La fonction applique :
         *   - anti-spam CAN
         *   - respect de l’état RUNNING
         *   - envoi via CanBooster_sendDccBit()
         * --------------------------------------------------------- */
        BoosterState_sendCan();

        // Log sécurisé (mode test uniquement)
        LOG_CRITICAL_DCC("CAN TX → bit envoyé");

        /* -----------------------------------------------------------
         * Cadence fixe 2 ms
         * Cette tâche n’est pas critique : un delay est acceptable.
         * --------------------------------------------------------- */
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}
