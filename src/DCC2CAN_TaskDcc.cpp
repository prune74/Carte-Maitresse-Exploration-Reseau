/*
 * DCC2CAN_TaskDcc.cpp
 *
 * Rôle
 * ----
 * Tâche FreeRTOS chargée de récupérer les événements DCC provenant :
 *   • du décodeur réel (ISR)
 *   • du simulateur FakeDCC (mode test)
 *
 * Cette tâche constitue l’entrée du flux DCC dans la logique interne :
 *   • elle lit tous les événements disponibles dans la queue
 *   • elle transmet chaque événement à BoosterState_updateFromDcc()
 *   • elle active FakeDCC si le système fonctionne en mode test
 *
 * Contraintes temps réel
 * ----------------------
 *   • aucun vTaskDelay() (risque de perte de bits)
 *   • queue vidée entièrement à chaque itération
 *   • logs critiques uniquement en mode test
 */

#include "DCC2CAN_TaskDcc.h"
#include "DCC2CAN_DccDecoder.h"
#include "DCC2CAN_State.h"
#include "DCC2CAN_FakeDcc.h"
#include "Debug.h"

// Indique si le système fonctionne en mode test
extern bool g_isTestMode;

void taskDcc(void *pv)
{
    (void)pv;

    LOG_INFO("Tâche DCC démarrée (mode %s)",
             g_isTestMode ? "test" : "réel");

    /* -----------------------------------------------------------------------
       INITIALISATION DU MODE TEST
       --------------------------------------------------------------------- */
    if (g_isTestMode)
    {
        FakeDcc_begin();
        LOG_INFO("FakeDCC → initialisé");
    }
    else
    {
        LOG_INFO("DCC réel → ISR active");
    }

    DccEvent ev;

    for (;;)
    {
        /* -------------------------------------------------------------------
           MODE TEST : génération d’un bit synthétique
           ----------------------------------------------------------------- */
        if (g_isTestMode)
        {
            FakeDcc_tick();
            LOG_CRITICAL_DCC("FakeDCC → tick");
        }

        /* -------------------------------------------------------------------
           TRAITEMENT DE TOUS LES ÉVÉNEMENTS DCC DISPONIBLES
           ----------------------------------------------------------------- */
        while (DccDecoder_getEvent(ev))
        {
            // Mise à jour de l’état global + envoi immédiat du CUTOUT si besoin
            BoosterState_updateFromDcc(ev);

            // Log sécurisé (mode test uniquement)
            LOG_CRITICAL_DCC("DCC RX → bit=%u phase=%u dt=%lu type=%u",
                             ev.bit,
                             ev.phase,
                             (unsigned long)ev.dt_us,
                             ev.type);
        }

        /* -------------------------------------------------------------------
           YIELD NON BLOQUANT
           ----------------------------------------------------------------- */
        taskYIELD(); // laisse tourner les autres tâches sans bloquer
    }
}
