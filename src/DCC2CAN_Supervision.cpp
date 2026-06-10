/*
 * DCC2CAN_Supervision.cpp
 *
 * Tâche FreeRTOS chargée de superviser la présence du signal DCC.
 *
 * Son rôle est simple mais essentiel :
 *   - attendre le premier événement DCC avant d'activer la supervision
 *   - surveiller l’absence prolongée d’événements (perte du signal)
 *   - gérer la transition entre RUNNING et RECOVERY
 *   - rétablir l’état RUNNING lorsque le signal revient
 *
 * Cette supervision fonctionne indépendamment du décodeur et du CAN.
 * Elle s’appuie uniquement sur :
 *   - g_state.lastEventTime
 *   - g_state.status
 */

#include "DCC2CAN_Supervision.h"
#include "Variables.h"
#include "Debug.h"

/* ---------------------------------------------------------------------------
   TÂCHE DE SUPERVISION DU SIGNAL DCC
   ---------------------------------------------------------------------------
   Cette tâche tourne toutes les 20 ms et vérifie le temps écoulé depuis
   le dernier événement DCC. Elle applique ensuite la logique de failsafe.
--------------------------------------------------------------------------- */
void taskSupervision(void *pv)
{
    (void)pv;

    LOG_INFO("DCC2CAN → tâche Supervision démarrée (20 ms)");

    bool supervisionActive = false;

    for (;;)
    {
        /* ---------------------------------------------------------------
           ACTIVATION DE LA SUPERVISION
           --------------------------------------------------------------- */
        if (!supervisionActive)
        {
            if (g_state.lastEventTime != 0)
            {
                supervisionActive = true;
                LOG_INFO("DCC2CAN → Supervision activée (premier événement reçu)");
            }

            vTaskDelay(pdMS_TO_TICKS(20));
            continue;
        }

        /* ---------------------------------------------------------------
           SUPERVISION NORMALE
           --------------------------------------------------------------- */
        uint32_t now = millis();
        uint32_t dt = now - g_state.lastEventTime;

        // Perte du signal DCC
        if (dt > 800 && g_state.status == BSTATE_RUNNING)
        {
            LOG_WARN("⚠️  DCC LOST → aucun événement depuis %u ms", dt);
            g_state.status = BSTATE_RECOVERY;
        }

        // Signal revenu
        if (dt <= 800 && g_state.status == BSTATE_RECOVERY)
        {
            LOG_INFO("✅ DCC RECOVERED → signal revenu");
            g_state.status = BSTATE_RUNNING;
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
