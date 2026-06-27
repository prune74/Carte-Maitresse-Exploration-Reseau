#pragma once
#include <Arduino.h>

/*
 * 🎯 Rôle
 * Déclaration de la tâche FreeRTOS dédiée à la supervision du réseau ERM.
 *
 * La tâche effectue :
 *   • la surveillance des Canton Controllers
 *   • la supervision du bus CAN
 *   • la mise à jour WebSocket
 */

class ERM_Task
{
public:
    static void begin();

private:
    static void taskLoop(void *pvParameters);
};
