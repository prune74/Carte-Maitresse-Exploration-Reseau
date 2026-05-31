#pragma once
#include <Arduino.h>

/* ============================================================
   🟥 LED STOP (nouveau Discovery 2026)
   ------------------------------------------------------------
   - Rouge
   - Allumée lorsque STOP global (CAN 0x201) est actif
   - Permet un diagnostic immédiat sans UI Web
   ============================================================ */
static const gpio_num_t PIN_LED_STOP = GPIO_NUM_25;

/* ============================================================
   🟦 Bouton CLEAR STOP (nouveau Discovery 2026)
   ------------------------------------------------------------
   - Bouton physique
   - Appui court → envoie CLEAR STOP (CAN 0x202)
   - Pull-up interne recommandé
   - Permet de lever STOP même sans interface Web
   ============================================================ */
static const gpio_num_t PIN_BTN_CLEAR_STOP = GPIO_NUM_26;