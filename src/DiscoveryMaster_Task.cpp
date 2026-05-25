/*
DiscoveryMaster_Task.cpp / .h

🎯 Rôle
Gestion des tâches FreeRTOS internes du module SAMain.
Ce module crée et exécute des tâches système destinées à effectuer des actions
périodiques liées au fonctionnement du réseau Discovery (supervision, maintenance,
sauvegarde, etc.).

📌 Fonctionnement
- begin() crée une tâche FreeRTOS dédiée (task1).
- task1() s’exécute en boucle toutes les X millisecondes (60 secondes par défaut).
- Le code initial prévoyait l’envoi périodique d’une commande CAN 0xBF (SAVE SETTINGS)
  à tous les satellites, mais cette partie est actuellement commentée.
- Le module sert aujourd’hui de base pour ajouter :
  • heartbeat Discovery
  • supervision globale
  • actions périodiques système
  • watchdog SAMain

📌 Particularités
- Fonctionne indépendamment du DCC_CAN-Booster.
- Ne contient aucune logique métier critique pour l’instant.
- Peut être enrichi dans Discovery 2026 pour la supervision avancée.
*/

#include "DiscoveryMaster_Task.h"

void DiscoveryMaster_Task::begin()
{
  TaskHandle_t task1Handle = NULL;
  xTaskCreate(task1, "Task1", 3 * 1024, NULL, 2, &task1Handle);
}

//--- Task1 : Envoi à tous les sat l'ordre de sauvegarde sur flash des settings
void DiscoveryMaster_Task::task1(void *pvParameters)
{
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();

  uint32_t tempo = 60UL * 1000UL;

  for (;;)
  {
    // CANMessage frameOut;
    // for (byte i = 0; i < 20; i++)
    // {
    //   frameOut.id |= 3 << 27; // Priorite 0, 1 ou 2
    //   frameOut.id |= 254 << 19;    // ID expediteur
    //   frameOut.id |= i << 11;      // ID destinataire
    //   frameOut.id |= 0xBF << 3;    // fonction
    //   frameOut.ext = true;
    //   delay(10);
    //   ACAN_ESP32::can.tryToSend (frameOut);
    // }
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(tempo)); // toutes les x ms
  }
}
