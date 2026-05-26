#pragma once
#include "DCC2CAN_TaskDcc.h"
#include "DCC2CAN_TaskCan.h"
#include "DCC2CAN_Supervision.h"
#include "DCC2CAN_DccDecoder.h"
#include "DCC2CAN_CanBooster.h"
#include "DCC2CAN_Cli.h"

void Booster_setup();
void Booster_loop();
