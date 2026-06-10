/*
 * DCC2CAN_TaskDcc.cpp
 *
 * Tâche FreeRTOS chargée de récupérer les événements DCC, qu’ils proviennent :
 *   - du décodeur réel (ISR)
 *   - du simulateur FakeDCC (mode test)
 *
 * Cette tâche constitue le point d’entrée du flux DCC dans la logique interne :
 *   - elle lit les événements DCC depuis la queue
 *   - elle transmet chaque événement à BoosterState_updateFromDcc()
 *   - elle active FakeDCC si le système fonctionne en mode test
 *
 * La cadence fixe de 1 ms garantit une réactivité optimale.
 */

#include "DCC2CAN_TaskDcc.h"
#include "DCC2CAN_DccDecoder.h"
#include "DCC2CAN_State.h"
#include "DCC2CAN_FakeDcc.h"
#include "Debug.h"

// Indique si le système fonctionne en mode test (FakeDCC)
extern bool g_isTestMode;

/* ---------------------------------------------------------------------------
   TÂCHE DCC
   ---------------------------------------------------------------------------
   Cette tâche tourne toutes les 1 ms et :
     - génère un tick FakeDCC si nécessaire
     - lit les événements DCC (réels ou simulés)
     - met à jour l’état logique du Booster
--------------------------------------------------------------------------- */
void taskDcc(void *pv)
{
    (void)pv;

    LOG_INFO("Tâche DCC démarrée → cadence 1 ms");

    /* ---------------------------------------------------------------
       INITIALISATION DU MODE TEST
       --------------------------------------------------------------- */
    if (g_isTestMode)
    {
        FakeDcc_begin();
        LOG_INFO("FakeDCC → initialisé (mode test)");
    }
    else
    {
        LOG_INFO("DCC réel → ISR active (mode réel)");
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
            LOG_VERBOSE("FakeDCC → tick dans taskDcc");
        }

        /* -----------------------------------------------------------
           LECTURE D’UN ÉVÉNEMENT DCC
           ----------------------------------------------------------- */
        if (DccDecoder_getEvent(ev))
        {
            BoosterState_updateFromDcc(ev);

            LOG_VERBOSE("DCC RX → bit=%u phase=%u dt=%lu type=%u",
                        ev.bit,
                        ev.phase,
                        (unsigned long)ev.dt_us,
                        ev.type);
        }

        vTaskDelay(pdMS_TO_TICKS(1));  // cadence fixe 1 ms
    }
}
