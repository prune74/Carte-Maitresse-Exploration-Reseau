/*
 * DCC2CAN_FakeDcc.cpp
 *
 * Rôle
 * ----
 * Module de simulation du signal DCC.
 *
 * Ce composant génère artificiellement des événements DCC afin de permettre
 * le test complet du pipeline DCC2CAN sans matériel réel. Il remplace
 * entièrement l’ISR du décodeur lorsque le système fonctionne en mode TEST.
 *
 * La simulation repose sur :
 *   • la même queue FreeRTOS que le décodeur réel
 *   • un tick régulier générant des bits DCC synthétiques
 *
 * La simulation est volontairement minimaliste : elle génère un flux régulier
 * de bits 1, suffisant pour valider :
 *   • le pipeline DCC → état → CAN
 *   • le Booster
 *   • les tâches FreeRTOS
 *
 * Remarque
 * --------
 * Le simulateur ne génère PAS de cutout. Le cutout est un phénomène analogique
 * complexe, inutile pour les tests unitaires du pipeline numérique.
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
 * INITIALISATION DE LA SIMULATION
 *
 * La simulation utilise la même queue que le décodeur réel.
 * Si la queue n’existe pas encore, la simulation ne peut pas fonctionner.
 * ------------------------------------------------------------------------- */
void FakeDcc_begin()
{
    q = DccDecoder_getQueue();

    if (q == nullptr)
    {
        LOG_ERROR("FakeDCC → queue DCC introuvable !");
    }
    else
    {
        LOG_INFO("FakeDCC → simulation DCC activée");
    }
}

/* ---------------------------------------------------------------------------
 * TICK DE SIMULATION
 *
 * Cette fonction doit être appelée régulièrement (par taskDcc en mode test).
 * Elle génère un bit DCC synthétique toutes les ~60 µs, ce qui correspond
 * à un bit 1 NMRA.
 *
 * Contraintes :
 *   • aucun traitement lourd
 *   • aucun delay
 *   • logs protégés via LOG_CRITICAL_DCC
 * ------------------------------------------------------------------------- */
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
    ev.bit   = 1;
    ev.type  = DCC_EVT_BIT;

    // Envoi non bloquant dans la queue
    xQueueSend(q, &ev, 0);

    // Log sécurisé : actif uniquement en mode test + VERBOSE
    LOG_CRITICAL_DCC("FakeDCC → BIT 1 simulé");
}
