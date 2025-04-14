# 🛰️ Projet ESP32 MQTTs – Supervision de Machines Industrielles

Ce projet permet à un **ESP32** de se connecter à un **broker MQTT sécurisé (MQTTs)** afin de publier régulièrement des mesures issues d’une machine industrielle (armoire, four, frigo, etc.). Il inclut également la réception de commandes via MQTT.

---

## 🔧 Fonctionnalités principales

- Connexion sécurisée au Wi-Fi et au broker MQTT via TLS.
- Publication périodique de données au format JSON (via `ArduinoJson`).
- Abonnement à un topic pour recevoir des commandes.
- Adaptation dynamique selon le type de machine (armoire, four, frigo...).
- Intégration de capteurs analogiques (via `ADS1115`) et d'entrées numériques.
- Configuration centralisée dans `device_config.h` et `secrets.h`.

---

## 📁 Structure du projet

```bash
.
├── src/
│   ├── main.cpp            # Fichier principal Arduino (PlatformIO ou IDE Arduino)
│   ├── device_config.h     # Paramètres matériels de la machine (type, capteurs, I/O)
│   └── secrets.h           # Identifiants WiFi et certificats TLS (à créer manuellement)
├── include/
│   └── ...
├── certs/
│   ├── ca.crt              # Certificat racine du broker MQTT
│   └── ...
├── README.md               # Ce fichier
└── platformio.ini          # Configuration PlatformIO (si utilisé)
```

---

## 🔒 Fichier `secrets.h` (à créer)

Ce fichier contient les informations sensibles de connexion :

```cpp
#pragma once

const char* ssid = "VOTRE_SSID_WIFI";
const char* password = "VOTRE_MOT_DE_PASSE_WIFI";

const char* mqtt_server = "192.168.1.100";  // Adresse IP ou nom DNS du broker
const int mqtt_port = 8883;                // Port TLS

const char* mqtt_user = "utilisateur";
const char* mqtt_pass = "motdepasse";

// Certificat du broker (fichier .crt converti en chaîne C)
const char* ca_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIID...
...contenu du certificat...
-----END CERTIFICATE-----
)EOF";
```

---

## 🚀 Démarrage rapide

### 1. Pré-requis

- Carte **ESP32**
- Broker **Mosquitto** avec TLS et mot de passe (voir [guide d’installation ici](https://mosquitto.org/man/mosquitto.conf-5.html))
- Bibliothèques Arduino :
  - `PubSubClient`
  - `ArduinoJson`
  - `Adafruit_ADS1X15`
  - `WiFiClientSecure`

### 2. Compilation

Avec PlatformIO :
```bash
pio run
pio upload
```

Avec Arduino IDE :
- Sélectionner la carte "ESP32 Dev Module"
- Ajouter manuellement les bibliothèques
- Uploader le code

---

## 🔁 Boucle principale

- Toutes les `10 secondes` :
  - Lecture des capteurs (PT100, humidité, tension, etc.)
  - Construction d’un **JSON structuré**
  - Envoi sur le topic : `test/topic`

Exemple de JSON publié :
```json
{
  "device_id": "ARM1",
  "type": "armoire",
  "status": "OK",
  "firmware": "1.0.0",
  "uptime": "12345",
  "pt100_1": {
    "value": 23.5,
    "unit": "°C",
    "type": "temperature"
  },
  ...
}
```

---

## 📥 Commandes via MQTT

Le code s'abonne automatiquement au topic `test/cmd`. Les commandes reçues peuvent être affichées ou traitées selon le besoin (fonction `callback()` à enrichir).

---

## 🛠️ Configuration machine

Le fichier `device_config.h` permet d’adapter le programme à différents types de machines :

```cpp
const char* machine_type = "armoire"; // "four", "frigo", etc.
```

Chaque type dispose de :
- Ses capteurs spécifiques
- Sa logique d'initialisation (`initMachine`)
- Ses données personnalisées dans `fillMachineData`

---

## ✅ Extensions possibles

- Support de la reconnection automatique au WiFi
- Gestion de l’OTA (mise à jour firmware via réseau)
- Affichage sur écran OLED/TFT
- Ajout de watchdog

---

## 📬 Exemple de configuration côté Mosquitto

```conf
listener 8883
cafile /etc/mosquitto/certs/ca.crt
require_certificate false
allow_anonymous false
password_file /etc/mosquitto/passwd
```

---

## 📖 Auteurs et licence

Projet développé dans le cadre d’une supervision industrielle personnalisable par [Votre Nom / Équipe].

Licence : MIT (ou autre selon votre choix)

