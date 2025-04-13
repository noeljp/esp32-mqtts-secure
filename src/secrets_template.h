// secrets_template.h
// 👉 Copiez ce fichier en "secrets.h" et remplissez vos valeurs
// ⚠️ Ne pas versionner "secrets.h" dans Git !

#pragma once

// Informations Wi-Fi
const char* wifi_ssid = "VOTRE_SSID";
const char* wifi_password = "VOTRE_MOT_DE_PASSE";

// Informations MQTT
const char* mqtt_server = "192.168.0.X";     // Adresse IP du serveur Mosquitto
const int mqtt_port = 8883;
const char* mqtt_user = "espuser";
const char* mqtt_pass = "motdepasse";

// Certificat CA (le ca.crt converti en bloc string)
const char* ca_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIC...VOTRE_CERTIFICAT_CA...
... (collez ici le contenu complet de votre ca.crt) ...
-----END CERTIFICATE-----
)EOF";
