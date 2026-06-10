/*
 * DCC2CAN_TaskCan.cpp
 *
 * 🎯 Rôle
 * Tâche FreeRTOS responsable de l’émission des trames CAN contenant
 * l’état DCC courant (bit logique + phase). Cette tâche constitue le
 * lien entre :
 *   • l’état logique du Booster (BoosterState)
 *   • le bus CAN Booster (via CanBooster_sendDccBit)
 *
 * Le principe est simple :
 *   - toutes les 2 ms, la tâche demande à BoosterState d’envoyer
 *     le bit DCC courant sur le bus CAN0
 *   - BoosterState_sendCan() applique sa logique interne :
 *        • anti-spam (envoi uniquement si changement)
 *        • respect de l’état RUNNING / STOP
 *
 * 🛡️ Sécurité temps réel
 * Cette tâche n’est PAS critique (contrairement à taskDcc). Elle tourne
 * à faible priorité et ne doit jamais bloquer la chaîne DCC → état.
 *
 * Les logs sont protégés via LOG_CRITICAL_DCC :
 *   • actifs uniquement en mode test + DEBUG_VERBOSE
 *   • totalement désactivés en mode réel
 */

#include "DCC2CAN_TaskCan.h"
#include "DCC2CAN_State.h"
#include "Debug.h"

/* ---------------------------------------------------------------------------
 * 🧵 TÂCHE CAN (TX)
 *
 * Cadence : 2 ms
 * Priorité : inférieure à taskDcc
 *
 * Cette tâche :
 *   - appelle BoosterState_sendCan() pour émettre le bit courant
 *   - laisse la logique interne décider si un envoi est nécessaire
 *   - ne fait aucun traitement lourd
 *   - ne bloque jamais la chaîne DCC
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

        // Log sécurisé : actif uniquement en mode test + VERBOSE
        LOG_CRITICAL_DCC("CAN TX → bit envoyé");

        /* -----------------------------------------------------------
         * Cadence fixe 2 ms
         * Cette tâche n’est pas critique : un delay est acceptable.
         * --------------------------------------------------------- */
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}
