/*
 * DCC2CAN_TaskDcc.cpp
 *
 * Tâche FreeRTOS chargée de récupérer les événements DCC, qu’ils proviennent :
 *   - du décodeur réel (ISR)
 *   - du simulateur FakeDCC (mode test)
 *
 * Cette tâche constitue le point d’entrée du flux DCC dans la logique interne :
 *   - elle lit tous les événements DCC disponibles dans la queue
 *   - elle transmet chaque événement à BoosterState_updateFromDcc()
 *   - elle active FakeDCC si le système fonctionne en mode test
 *
 * ⚠️ Version optimisée temps réel :
 *   - aucun vTaskDelay() (risque de perte de bits)
 *   - queue vidée entièrement à chaque itération
 *   - logs critiques protégés via LOG_CRITICAL_DCC
 */

#include "DCC2CAN_TaskDcc.h"
#include "DCC2CAN_DccDecoder.h"
#include "DCC2CAN_State.h"
#include "DCC2CAN_FakeDcc.h"
#include "Debug.h"

// Indique si le système fonctionne en mode test (FakeDCC)
extern bool g_isTestMode;

void taskDcc(void *pv)
{
    (void)pv;

    LOG_INFO("Tâche DCC démarrée (mode %s)",
             g_isTestMode ? "test" : "réel");

    /* ---------------------------------------------------------------
       INITIALISATION DU MODE TEST
       --------------------------------------------------------------- */
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
        /* -----------------------------------------------------------
           MODE TEST : génération d’un bit synthétique
           ----------------------------------------------------------- */
        if (g_isTestMode)
        {
            FakeDcc_tick();
            LOG_CRITICAL_DCC("FakeDCC → tick");
        }

        /* -----------------------------------------------------------
           TRAITEMENT DE TOUS LES ÉVÉNEMENTS DCC DISPONIBLES
           ----------------------------------------------------------- */
        while (DccDecoder_getEvent(ev))
        {
            BoosterState_updateFromDcc(ev);

            LOG_CRITICAL_DCC("DCC RX → bit=%u phase=%u dt=%lu type=%u",
                             ev.bit,
                             ev.phase,
                             (unsigned long)ev.dt_us,
                             ev.type);
        }

        /* -----------------------------------------------------------
           YIELD NON BLOQUANT
           ----------------------------------------------------------- */
        taskYIELD();  // laisse tourner les autres tâches sans bloquer
    }
}
