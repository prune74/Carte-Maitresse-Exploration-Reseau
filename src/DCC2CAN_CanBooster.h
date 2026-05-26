#pragma once
#include <ACAN_ESP32.h>
#include "DCC2CAN_DccDecoder.h"

void CanBooster_begin();
bool CanBooster_sendDccBit(uint8_t bit, uint8_t phase);
