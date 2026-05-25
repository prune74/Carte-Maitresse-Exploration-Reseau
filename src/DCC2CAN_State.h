#pragma once
#include "DCC2CAN_DccDecoder.h"
#include "DCC2CAN_CanBooster.h"
#include "DCC2CAN_Config.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

enum BoosterSupervisionState : uint8_t
{
    BSTATE_RUNNING = 0,
    BSTATE_DCC_LOST = 1,
    BSTATE_OVERCURRENT = 2,
    BSTATE_RECOVERY = 3
};

struct BoosterRuntimeState
{
    DccEvent lastEvent;
    BoosterSupervisionState status;
    uint32_t lastEventTime;
};

extern volatile bool canMonitorEnabled;
extern volatile int32_t canMonitorFilter;
extern volatile BoosterRuntimeState g_state;
extern SemaphoreHandle_t gStateUpdateMutex;

void BoosterState_init();
void BoosterState_updateFromDcc(const volatile DccEvent &ev);
void BoosterState_sendCan();
void BoosterState_supervise();
