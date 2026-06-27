# DCC2CAN — Canton Controller 2026  
Convertisseur DCC → CAN Booster (TX only)

## 🎯 Rôle du module
DCC2CAN est un module firmware du projet **Canton Controller 2026**.  
Il convertit en temps réel le signal **DCC logique** provenant de LaBox en trames **CAN Booster** destinées aux boosters Canton Controller.

Le module :
- décode le signal DCC (bit, phase, timing)
- supervise la présence du flux DCC (failsafe)
- transmet le bit DCC courant sur le bus **CAN interne de l’ESP32**
- fonctionne **uniquement en émission** (TX only)

Aucune réception CAN, aucune télémétrie, aucun RailCom.

---

## 🧩 Architecture générale
```text
DCC (LaBox)
    │
    ▼
DccDecoder (ISR)
    │  événements DCC
    ▼
taskDcc (1 ms)
    │  met à jour l’état
    ▼
DCC2CAN_State
    │  bit + phase + supervision
    ▼
taskCan (2 ms)
    │  envoi CAN
    ▼
CanBooster (ACAN_ESP32)
    │
    ▼
Bus CAN Booster (TX only)
```
---

## 📦 Sous-modules

### **1. DccDecoder (ISR)**
- Décode les fronts du signal DCC logique (via PIN_DCC_IN)
- Produit des événements DCC (bit, phase, durée)
- Stocke les événements dans une file thread-safe

### **2. taskDcc (1 ms)**
- Lit les événements DCC décodés
- Met à jour l’état global (`g_state.lastEvent`)
- Latence minimale, aucune logique métier

### **3. DCC2CAN_State**
- Stocke :
  - dernier événement DCC
  - timestamp
  - état de supervision (RUNNING, DCC_LOST, RECOVERY)
- Mutex pour compatibilité dual‑core ESP32
- Supervision du flux DCC (timeout + cooldown)
- Envoi CAN conditionnel (désactivé en failsafe)

### **4. taskCan (2 ms)**
- Envoie le bit DCC courant + phase via `CanBooster_sendDccBit()`
- Débit stable et régulier

### **5. CanBooster (ACAN_ESP32)**
- Initialise le CAN interne ESP32
- Envoie les trames CAN standard (11 bits)
- TX only

### **6. taskSupervision (20 ms)**
- Vérifie la présence du flux DCC
- Active le failsafe en cas de perte
- Gère le retour à RUNNING

---

## 🔌 Brochage (Pins.h)

| Fonction       | Broche ESP32 |
|----------------|--------------|
| Entrée DCC     | GPIO 27      |
| CAN TX         | GPIO 5       |
| CAN RX         | GPIO 4       |
| LED debug      | GPIO 2       |

---

## ⚙️ Tâches FreeRTOS

| Tâche            | Période | Rôle                            |
|------------------|---------|---------------------------------|
| `taskDcc`        | 1 ms    | Consommation des événements DCC |
| `taskCan`        | 2 ms    | Envoi CAN Booster               |
| `taskSupervision`| 20 ms   | Failsafe DCC                    |

---

## 🚦 Supervision DCC

Le module détecte :
- **perte du signal DCC** (timeout)
- **cooldown** avant recovery
- **retour automatique à RUNNING**

En cas de DCC_LOST ou RECOVERY :
- **aucune trame CAN n’est envoyée**

---

## 🧪 Dépendances

- **ACAN_ESP32** → driver CAN interne ESP32  
- **FreeRTOS** → multitâche  
- **Arduino** → environnement runtime  

---

## 📝 Notes importantes

- Le bus **CAN Booster est unidirectionnel** pour DCC2CAN (TX only).  
- Aucune réception CAN n’est utilisée ni prévue.  
- Le module ne gère **ni RailCom**, **ni télémétrie**, **ni surintensité**.  
- Le firmware est optimisé pour un fonctionnement temps réel strict.

---

## 📁 Structure du dossier
```text
SCR/
├── DCC2CAN_main.cpp
├── Pins.h
├── DCC2CAN_DccDecoder.*
├── DCC2CAN_TaskDcc.*
├── DCC2CAN_TaskCan.*
├── DCC2CAN_Supervision.*
├── DCC2CAN_State.*
├── DCC2CAN_CanBooster.*
└── DCC2CAN_README.md
```
---

## ✔️ Statut
Module **stable**, **minimal**, **fiable**, utilisé dans Canton Controller 2026 pour la conversion DCC → CAN Booster.