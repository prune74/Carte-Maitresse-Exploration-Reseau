/*
DiscoveryMaster_Satellite.cpp / .h

🎯 Rôle
Représentation logicielle d’un satellite Discovery.
Chaque instance correspond à un satellite physique et stocke ses informations
(ID, adresse IP, état). Le module inclut également un watchdog interne.

📌 Fonctionnement
- Le constructeur initialise l’ID du satellite et son adresse IP.
- begin() crée une tâche FreeRTOS (watchDog) dédiée à la surveillance du satellite.
- watchDog() s’exécute périodiquement (toutes les 100 ms) et constitue un point
  d’entrée idéal pour :
  • surveiller l’activité du satellite
  • détecter les timeouts
  • gérer les états online/offline
  • déclencher des actions automatiques

📌 Particularités
- Le watchdog est actuellement vide : il sert de squelette pour les futures
  fonctionnalités Discovery 2026.
- Le module est conçu pour évoluer vers une supervision complète des satellites.
- Fonctionne indépendamment du module DCC_CAN-Booster.
*/

#include "DiscoveryMaster_Satellite.h"

DiscoveryMaster_Satellite::DiscoveryMaster_Satellite()
{
    m_idNode = NO_ID; // optionnel mais propre
    m_ip[0] = m_ip[1] = m_ip[2] = m_ip[3] = 0;
}

// --- IMPORTANT : rendre id() const ---
uint16_t DiscoveryMaster_Satellite::id() const
{
    return m_idNode;
}

void DiscoveryMaster_Satellite::id(uint16_t id)
{
    this->m_idNode = id;
}

void DiscoveryMaster_Satellite::begin()
{
    TaskHandle_t task1Handle = NULL;
    xTaskCreate(watchDog, "watchDog", 3 * 1024, NULL, 2, &task1Handle);
}

//--- Task1 : Envoi à tous les sat l'ordre de sauvegarde sur flash des settings
void DiscoveryMaster_Satellite::watchDog(void *pvParameters)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    uint32_t tempo = 100UL;

    for (;;)
    {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(tempo)); // toutes les x ms
    }
}
