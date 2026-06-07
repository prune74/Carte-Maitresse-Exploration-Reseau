#pragma once
#include <Arduino.h>
#include "DCC2CAN_DccDecoder.h"

extern bool DCC_FAKE_MODE;

void FakeDcc_begin();
void FakeDcc_tick();
