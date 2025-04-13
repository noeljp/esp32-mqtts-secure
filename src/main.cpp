#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>  // à placer en haut de ton fichier
#include "secrets.h" 
#include "device_config.h"


#define MQTT_MAX_PACKET_SIZE 512
#define MQTT_KEEP_ALIVE 60  // 60 secondes
#define MQTT_RECONNECT_TIMEOUT 5  // 5 secondes
#define MQTT_PUBLISH_INTERVAL 5000  // 1 seconde  


// MQTT client
WiFiClientSecure espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connexion à ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("Connecté au WiFi !");
  Serial.print(" IP : ");
  Serial.print(WiFi.localIP());
  Serial.print(" Server : ");
  Serial.print(mqtt_server);
  Serial.print("Port : ");
  Serial.println(mqtt_port);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("📩 Message reçu sur le topic ");
  Serial.print(topic);
  Serial.print(" : ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connexion au broker MQTT...");
    if (client.connect(device_id, mqtt_user, mqtt_pass)) {
      Serial.println("connecté !");
      client.subscribe(mqtt_subscribe_topic);  // <--- ici
      Serial.print("Abonné à : ");
      Serial.println(mqtt_subscribe_topic);
    } else {
      Serial.print("échec, rc=");
      Serial.print(client.state());
      Serial.println(" nouvelle tentative dans 5 secondes");
      delay(5000);
    }
  }
}

// Voici une version simplifiée de ce que fait PlatformIO / Arduino sous le capot :
//  int main() {
//    init();           // Initialisation matérielle
//    setup();          // Appel une seule fois
//    while (true) {
//      loop();         // Appelé à l’infini
//    }
//  }

// Fonction d'initialisation arduino
// (appelée une seule fois au démarrage)
void setup() {
  Serial.begin(115200);

  setup_wifi();
  // Charger le certificat
  espClient.setCACert(ca_cert);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);  
  // Initialiser les broches
  initMachine();
}

// Fonction principale Arduino(appelée en boucle)
// Elle est exécutée en continu après le setup()
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg >= MQTT_PUBLISH_INTERVAL) {
    lastMsg = now;

    JsonDocument doc;

    doc["device_id"] = String(device_id);
    doc["type"] = String(machine_type);
    doc["status"] = String(getStatus());
    doc["firmware"] = String(firmware_version);
    doc["uptime"] = String(millis() / 1000); // Uptime en secondes

    fillMachineData(doc);

    String payload;
    serializeJson(doc, payload);

    Serial.print("📤 JSON payload : ");
    Serial.println(payload);
    Serial.print("🧮 Taille JSON : ");
    Serial.println(measureJson(doc));


    if (client.publish(mqtt_publish_topic, payload.c_str())) {
      Serial.println("✅ Publication MQTT réussie !");
    } else {
      Serial.println("❌ Échec de publication MQTT !");
    }
  }
}

