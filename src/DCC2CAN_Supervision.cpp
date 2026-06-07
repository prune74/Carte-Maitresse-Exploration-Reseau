#include "DCC2CAN_Supervision.h"
#include "Debug.h"

void taskSupervision(void *pv) {
    (void)pv;

    LOG_INFO("DCC2CAN → tâche Supervision démarrée (20 ms)");

    static bool supervisionActive = false;

    for (;;) {

        // Tant qu'aucun événement DCC n'a été reçu → NE PAS superviser
        if (!supervisionActive) {
            if (g_state.lastEventTime != 0) {
                supervisionActive = true;
                LOG_INFO("DCC2CAN → Supervision activée (premier événement reçu)");
            }
            vTaskDelay(pdMS_TO_TICKS(20));
            continue;
        }

        // --- Supervision normale ---
        uint32_t now = millis();
        uint32_t dt = now - g_state.lastEventTime;

        if (dt > 800 && g_state.status == BSTATE_RUNNING) {
            LOG_WARN("⚠️  DCC LOST → aucun événement depuis %u ms", dt);
            g_state.status = BSTATE_RECOVERY;
        }

        if (dt <= 800 && g_state.status == BSTATE_RECOVERY) {
            LOG_INFO("✅ DCC RECOVERED → signal revenu");
            g_state.status = BSTATE_RUNNING;
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

