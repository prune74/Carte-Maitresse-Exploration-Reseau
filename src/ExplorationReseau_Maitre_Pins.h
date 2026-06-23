#pragma once
#include <Arduino.h>

/* ============================================================
   🟥 LED STOP — Carte Maîtresse Exploration Réseau
   ------------------------------------------------------------
   • LED rouge
   • Allumée lorsque le STOP global (CAN 0x201) est actif
   • Permet un diagnostic immédiat sans interface Web
   ============================================================ */
static const gpio_num_t PIN_LED_STOP = GPIO_NUM_25;

/* ============================================================
   🟦 Bouton CLEAR STOP — Carte Maîtresse Exploration Réseau
   ------------------------------------------------------------
   • Bouton physique
   • Appui court → envoie CLEAR STOP (CAN 0x202)
   • Pull-up interne recommandé
   • Permet de lever STOP même sans interface Web
   ============================================================ */
static const gpio_num_t PIN_BTN_CLEAR_STOP = GPIO_NUM_26;

/* ============================================================
   🟧 LED CC OFFLINE — Carte Maîtresse Exploration Réseau
   ------------------------------------------------------------
   • LED orange (ou autre couleur)
   • Allumée lorsqu’un CC du réseau devient OFFLINE
   • Indique une anomalie topologique détectée par ERS
   ============================================================ */
static const gpio_num_t PIN_LED_CC_OFFLINE = GPIO_NUM_27;
