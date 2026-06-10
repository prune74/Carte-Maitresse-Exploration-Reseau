/*
 * DCC2CAN_FakeDcc.cpp
 *
 * Module de simulation du signal DCC.
 *
 * Ce composant génère artificiellement des événements DCC afin de permettre
 * le test complet du pipeline DCC2CAN sans matériel réel. Il remplace
 * entièrement l’ISR du décodeur lorsque le système fonctionne en mode TEST.
 *
 * La simulation repose sur :
 *   - une queue FreeRTOS (identique à celle utilisée par le décodeur réel)
 *   - un tick régulier générant des bits DCC synthétiques
 *
 * Ce module est volontairement minimal : il ne simule qu’un flux de bits 1
 * réguliers, ce qui suffit pour valider le fonctionnement du Booster,
 * du CAN et des tâches FreeRTOS.
 */

#include "DCC2CAN_FakeDcc.h"
#include "DCC2CAN_DccDecoder.h"
#include "Variables.h"
#include "Debug.h"

// Activation/désactivation du mode simulation
bool DCC_FAKE_MODE = true;

// Queue d’événements DCC (partagée avec le décodeur réel)
static QueueHandle_t q = nullptr;

/* ---------------------------------------------------------------------------
   INITIALISATION DE LA SIMULATION
   ---------------------------------------------------------------------------
   La simulation utilise la même queue que le décodeur réel. Si la queue
   n’existe pas encore, la simulation ne peut pas fonctionner.
--------------------------------------------------------------------------- */
void FakeDcc_begin()
{
    q = DccDecoder_getQueue();

    if (q == nullptr)
        LOG_ERROR("FakeDCC → queue DCC introuvable !");
    else
        LOG_INFO("FakeDCC → simulation DCC activée");
}

/* ---------------------------------------------------------------------------
   TICK DE SIMULATION
   ---------------------------------------------------------------------------
   Cette fonction doit être appelée régulièrement (par exemple dans une tâche
   FreeRTOS). Elle génère un bit DCC synthétique toutes les ~60 µs, ce qui
   correspond à un bit 1 NMRA.
--------------------------------------------------------------------------- */
void FakeDcc_tick()
{
    if (!DCC_FAKE_MODE || q == nullptr)
        return;

    static uint32_t last = 0;
    uint32_t now = micros();

    // Génère un bit toutes les 60 µs
    if (now - last < 60)
        return;

    last = now;

    DccEvent ev;
    ev.dt_us = 60;
    ev.phase = 0;
    ev.bit = 1;
    ev.type = DCC_EVT_BIT;

    xQueueSend(q, &ev, 0);

    LOG_VERBOSE("FakeDCC → BIT 1 simulé");
}
