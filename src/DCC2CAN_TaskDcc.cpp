#include "DCC2CAN_TaskDcc.h"
#include "DCC2CAN_DccDecoder.h"
#include "DCC2CAN_State.h"
#include "DCC2CAN_FakeDcc.h"
#include "Debug.h"

void taskDcc(void *pv)
{
    (void)pv;

    LOG_INFO("Tâche DCC démarrée → cadence 1 ms");

    // 🔥 initialiser FakeDCC
    FakeDcc_begin();
    LOG_INFO("FakeDCC → initialisé dans taskDcc");

    DccEvent ev;

    for (;;)
    {
        // 🔥 Simulation DCC
        FakeDcc_tick();
        LOG_VERBOSE("FakeDCC → tick dans taskDcc");

        // Lecture d’un événement réel ou simulé
        if (DccDecoder_getEvent(ev))
        {
            BoosterState_updateFromDcc(ev);

            LOG_VERBOSE("DCC RX → bit=%u phase=%u dt=%lu type=%u",
                        ev.bit,
                        ev.phase,
                        (unsigned long)ev.dt_us,
                        ev.type);
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
