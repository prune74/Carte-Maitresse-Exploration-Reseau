#include "DCC2CAN_FakeDcc.h"
#include "Debug.h"

bool DCC_FAKE_MODE = true;   // Simulation activée par défaut

static QueueHandle_t q = nullptr;

void FakeDcc_begin()
{
    q = DccDecoder_getQueue();

    if (q == nullptr)
        LOG_ERROR("FakeDCC → queue DCC introuvable !");
    else
        LOG_INFO("FakeDCC → simulation DCC activée");
}

void FakeDcc_tick()
{
    if (!DCC_FAKE_MODE || q == nullptr)
        return;

    static uint32_t last = 0;
    uint32_t now = micros();

    // Simule un bit 1 toutes les 60 µs (timing NMRA)
    if (now - last < 60)
        return;

    last = now;

    DccEvent ev;
    ev.dt_us = 60;
    ev.phase = 0;
    ev.bit   = 1;
    ev.type  = DCC_EVT_BIT;

    xQueueSend(q, &ev, 0);

    LOG_VERBOSE("FakeDCC → BIT 1 simulé");
}
