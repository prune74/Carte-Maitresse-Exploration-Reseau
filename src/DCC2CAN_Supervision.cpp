/*
 * DCC2CAN_Supervision.cpp
 *
 * 🎯 Rôle
 * Tâche FreeRTOS chargée de superviser la présence du signal DCC.
 *
 * Cette supervision est indépendante du décodeur et du CAN. Elle repose
 * uniquement sur :
 *   • g_state.lastEventTime  → timestamp du dernier événement DCC
 *   • g_state.status         → état logique du Booster
 *
 * Objectifs :
 *   - attendre le premier événement avant d’activer la supervision
 *   - détecter une absence prolongée de signal (failsafe)
 *   - gérer la transition RUNNING → RECOVERY
 *   - repasser en RUNNING lorsque le signal revient
 *
 * 🛡️ Sécurité temps réel
 * Cette tâche tourne à faible priorité (20 ms). Elle ne doit jamais
 * bloquer ni interférer avec taskDcc ou taskCan.
 */

#include "DCC2CAN_Supervision.h"
#include "Variables.h"
#include "Debug.h"

/* ---------------------------------------------------------------------------
 * 🧵 TÂCHE DE SUPERVISION DU SIGNAL DCC
 *
 * Cadence : 20 ms
 * Priorité : faible
 *
 * Fonctionnement :
 *   - phase 1 : attendre le premier événement DCC
 *   - phase 2 : surveiller l’absence de signal
 *   - basculer entre RUNNING et RECOVERY selon le timeout
 * ------------------------------------------------------------------------- */
void taskSupervision(void *pv)
{
    (void)pv;

    LOG_INFO("DCC2CAN → tâche Supervision démarrée (20 ms)");

    bool supervisionActive = false;

    for (;;)
    {
        /* -----------------------------------------------------------
         * PHASE 1 — Attente du premier événement DCC
         * --------------------------------------------------------- */
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

        /* -----------------------------------------------------------
         * PHASE 2 — Supervision normale
         * --------------------------------------------------------- */
        uint32_t now = millis();
        uint32_t dt  = now - g_state.lastEventTime;

        /* -------------------------------
         * Perte du signal DCC
         * ----------------------------- */
        if (dt > 800 && g_state.status == BSTATE_RUNNING)
        {
            LOG_WARN("⚠️  DCC LOST → aucun événement depuis %u ms", dt);
            g_state.status = BSTATE_RECOVERY;
        }

        /* -------------------------------
         * Signal revenu
         * ----------------------------- */
        if (dt <= 800 && g_state.status == BSTATE_RECOVERY)
        {
            LOG_INFO("✅ DCC RECOVERED → signal revenu");
            g_state.status = BSTATE_RUNNING;
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
